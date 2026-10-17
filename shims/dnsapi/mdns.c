/*
 * DnsStartMulticastQuery / DnsStopMulticastQuery for Wine.
 *
 * Windows hands these to the DNS Client service; Wine has no mDNS backend and only
 * stubs them. This is a small continuous mDNS (RFC 6762) querier over Winsock,
 * IPv4 only. It shares UDP 5353 with avahi-daemon via SO_REUSEADDR, or falls back to
 * an ephemeral port and legacy unicast responses (RFC 6762 6.7) if 5353 is taken.
 *
 * Each response that answers the query is turned into a DNS_RECORDW list (answers plus
 * PTR/SRV/A/AAAA/TXT additionals) and passed to the callback. The list is copied with the
 * real dnsapi's DnsRecordSetCopyEx so the caller can release it with Wine's DnsFree.
 */
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <windns.h>
#include <stddef.h>

#define MDNS_PORT 5353
#define MDNS_GROUP 0xe00000fbu /* 224.0.0.251 */
#define MAX_IFACES 32
#define MAX_RECORDS 64
#define RESEND_MAX_MS 60000

extern HMODULE real_dnsapi;

struct query {
    PMDNS_QUERY_CALLBACK callback;
    void *context;
    PMDNS_QUERY_HANDLE handle;
    ULONG64 options;
    ULONG ifindex;
    WORD type;
    char name[DNS_MAX_NAME_BUFFER_LENGTH]; /* UTF-8, no trailing dot */
    HANDLE thread, stop;
    DWORD tid;
    volatile LONG self_free; /* stopped from inside the callback: thread frees q */
};

struct rr {
    char name[DNS_MAX_NAME_BUFFER_LENGTH];
    WORD type, section;
    DWORD ttl;
    const BYTE *rdata;
    WORD rdlen, rdoff; /* rdata offset in packet, for compressed names inside rdata */
};

static PDNS_RECORDW (WINAPI *pDnsRecordSetCopyEx)(PDNS_RECORDW, DNS_CHARSET, DNS_CHARSET);

static int names_equal(const char *a, const char *b)
{
    for (; *a && *b; a++, b++) {
        char x = *a, y = *b;
        if (x >= 'A' && x <= 'Z') x += 32;
        if (y >= 'A' && y <= 'Z') y += 32;
        if (x != y) return 0;
    }
    return !*a && !*b;
}

/* Decode a possibly-compressed name at off; returns the offset just past it, or -1 */
static int read_name(const BYTE *pkt, int len, int off, char *out, int outsz)
{
    int end = -1, n = 0, jumps = 0;
    while (off < len) {
        BYTE l = pkt[off];
        if (!l) {
            if (end < 0) end = off + 1;
            if (n) n--; /* drop trailing dot */
            out[n] = 0;
            return end;
        }
        if ((l & 0xc0) == 0xc0) {
            if (off + 1 >= len || ++jumps > 32) return -1;
            if (end < 0) end = off + 2;
            off = ((l & 0x3f) << 8) | pkt[off + 1];
            continue;
        }
        if (l & 0xc0 || off + 1 + l > len || n + l + 2 > outsz) return -1;
        memcpy(out + n, pkt + off + 1, l);
        n += l;
        out[n++] = '.';
        off += 1 + l;
    }
    return -1;
}

static int write_name(BYTE *buf, int bufsz, const char *name)
{
    int n = 0;
    while (*name) {
        const char *dot = strchr(name, '.');
        int l = dot ? (int)(dot - name) : (int)strlen(name);
        if (l < 1 || l > 63 || n + l + 2 > bufsz) return -1;
        buf[n++] = l;
        memcpy(buf + n, name, l);
        n += l;
        name += l + (dot ? 1 : 0);
    }
    buf[n++] = 0;
    return n;
}

static WCHAR *wdup(const char *s, char **heap)
{
    int n = MultiByteToWideChar(CP_UTF8, 0, s, -1, NULL, 0);
    WCHAR *w = (WCHAR *)*heap;
    MultiByteToWideChar(CP_UTF8, 0, s, -1, w, n);
    *heap += n * sizeof(WCHAR);
    return w;
}

/* Build one DNS_RECORDW (strings packed after it) for a parsed RR; NULL if unsupported */
static DNS_RECORDW *make_record(const BYTE *pkt, int len, const struct rr *r)
{
    char target[DNS_MAX_NAME_BUFFER_LENGTH];
    const BYTE *d = r->rdata;
    int datalen, strcount = 0;
    size_t size;
    DNS_RECORDW *rec;
    char *heap;

    switch (r->type) {
    case DNS_TYPE_A:
        if (r->rdlen != 4) return NULL;
        datalen = sizeof(DNS_A_DATA);
        break;
    case DNS_TYPE_AAAA:
        if (r->rdlen != 16) return NULL;
        datalen = sizeof(DNS_AAAA_DATA);
        break;
    case DNS_TYPE_PTR:
        if (read_name(pkt, len, r->rdoff, target, sizeof(target)) < 0) return NULL;
        datalen = sizeof(DNS_PTR_DATAW);
        break;
    case DNS_TYPE_SRV:
        if (r->rdlen < 7 || read_name(pkt, len, r->rdoff + 6, target, sizeof(target)) < 0) return NULL;
        datalen = sizeof(DNS_SRV_DATAW);
        break;
    case DNS_TYPE_TEXT:
        for (int i = 0; i < r->rdlen; i += 1 + d[i]) {
            if (i + 1 + d[i] > r->rdlen) return NULL;
            strcount++;
        }
        if (!strcount) return NULL;
        datalen = offsetof(DNS_TXT_DATAW, pStringArray) + strcount * sizeof(PWSTR);
        break;
    default:
        return NULL;
    }

    /* worst case: every byte of a name or TXT string becomes one WCHAR */
    size = offsetof(DNS_RECORDW, Data) + datalen + 8 + (strlen(r->name) + 1) * sizeof(WCHAR)
         + (sizeof(target) + r->rdlen + strcount) * sizeof(WCHAR);
    if (!(rec = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size))) return NULL;
    heap = (char *)&rec->Data + ((datalen + 7) & ~7);

    rec->pName = wdup(r->name, &heap);
    rec->wType = r->type;
    rec->wDataLength = datalen;
    rec->Flags.S.Section = r->section;
    rec->Flags.S.CharSet = DnsCharSetUnicode;
    rec->dwTtl = r->ttl;

    switch (r->type) {
    case DNS_TYPE_A:
        memcpy(&rec->Data.A.IpAddress, d, 4);
        break;
    case DNS_TYPE_AAAA:
        memcpy(&rec->Data.AAAA.Ip6Address, d, 16);
        break;
    case DNS_TYPE_PTR:
        rec->Data.PTR.pNameHost = wdup(target, &heap);
        break;
    case DNS_TYPE_SRV:
        rec->Data.SRV.wPriority = (d[0] << 8) | d[1];
        rec->Data.SRV.wWeight = (d[2] << 8) | d[3];
        rec->Data.SRV.wPort = (d[4] << 8) | d[5];
        rec->Data.SRV.pNameTarget = wdup(target, &heap);
        break;
    case DNS_TYPE_TEXT: {
        char s[256];
        rec->Data.TXT.dwStringCount = strcount;
        for (int i = 0, k = 0; i < r->rdlen; i += 1 + d[i], k++) {
            memcpy(s, d + i + 1, d[i]);
            s[d[i]] = 0;
            rec->Data.TXT.pStringArray[k] = wdup(s, &heap);
        }
        break;
    }
    }
    return rec;
}

static void handle_packet(struct query *q, const BYTE *pkt, int len)
{
    struct rr rrs[MAX_RECORDS];
    DNS_RECORDW *head = NULL, **tail = &head, *copy;
    int off = 12, qd, total, nrr = 0, answered = 0;
    char skip[DNS_MAX_NAME_BUFFER_LENGTH];

    if (len < 12 || !(pkt[2] & 0x80)) return; /* not a response */
    qd = (pkt[4] << 8) | pkt[5];
    total = ((pkt[6] << 8) | pkt[7]) + ((pkt[8] << 8) | pkt[9]) + ((pkt[10] << 8) | pkt[11]);
    int an = (pkt[6] << 8) | pkt[7];

    for (int i = 0; i < qd; i++) {
        if ((off = read_name(pkt, len, off, skip, sizeof(skip))) < 0 || off + 4 > len) return;
        off += 4;
    }
    for (int i = 0; i < total && nrr < MAX_RECORDS; i++) {
        struct rr *r = &rrs[nrr];
        if ((off = read_name(pkt, len, off, r->name, sizeof(r->name))) < 0 || off + 10 > len) return;
        r->type = (pkt[off] << 8) | pkt[off + 1];
        r->ttl = ((DWORD)pkt[off + 4] << 24) | (pkt[off + 5] << 16) | (pkt[off + 6] << 8) | pkt[off + 7];
        r->rdlen = (pkt[off + 8] << 8) | pkt[off + 9];
        r->rdoff = off + 10;
        r->rdata = pkt + off + 10;
        r->section = i < an ? DnsSectionAnswer : DnsSectionAddtional; /* sic, SDK spelling */
        off += 10 + r->rdlen;
        if (off > len) return;
        /* TTL 0 is a goodbye; reporting it would make the caller re-add a departing service */
        if (!r->ttl) continue;
        if (names_equal(r->name, q->name) && (r->type == q->type || q->type == DNS_TYPE_ALL)) {
            r->section = DnsSectionAnswer;
            answered = 1;
        }
        nrr++;
    }
    if (!answered) return;

    /* answers first, then the additionals that help resolve them */
    for (int pass = 0; pass < 2; pass++) {
        for (int i = 0; i < nrr; i++) {
            int is_answer = names_equal(rrs[i].name, q->name);
            DNS_RECORDW *rec;
            if (is_answer != !pass) continue;
            if (!(rec = make_record(pkt, len, &rrs[i]))) continue;
            *tail = rec;
            tail = &rec->pNext;
        }
    }
    if (!head) return;

    copy = pDnsRecordSetCopyEx(head, DnsCharSetUnicode, DnsCharSetUnicode);
    while (head) {
        DNS_RECORDW *next = head->pNext;
        HeapFree(GetProcessHeap(), 0, head);
        head = next;
    }
    if (copy && WaitForSingleObject(q->stop, 0) == WAIT_TIMEOUT) {
        DNS_QUERY_RESULT res = { DNS_QUERY_RESULTS_VERSION1, ERROR_SUCCESS, q->options, (PDNS_RECORD)copy, NULL };
        q->callback(q->context, q->handle, &res);
    }
}

static int get_ifaces(struct query *q, ULONG *addrs)
{
    ULONG size = 16384, n = 0;
    IP_ADAPTER_ADDRESSES *list = NULL;

    for (int tries = 0; tries < 3; tries++) {
        list = HeapAlloc(GetProcessHeap(), 0, size);
        if (!list) return 0;
        ULONG err = GetAdaptersAddresses(AF_INET, GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER, NULL, list, &size);
        if (err == ERROR_SUCCESS) break;
        HeapFree(GetProcessHeap(), 0, list);
        list = NULL;
        if (err != ERROR_BUFFER_OVERFLOW) return 0;
    }
    if (!list) return 0;
    for (IP_ADAPTER_ADDRESSES *a = list; a; a = a->Next) {
        if (a->OperStatus != IfOperStatusUp || a->IfType == IF_TYPE_SOFTWARE_LOOPBACK) continue;
        if (q->ifindex && a->IfIndex != q->ifindex) continue;
        for (IP_ADAPTER_UNICAST_ADDRESS *u = a->FirstUnicastAddress; u && n < MAX_IFACES; u = u->Next)
            if (u->Address.lpSockaddr->sa_family == AF_INET)
                addrs[n++] = ((struct sockaddr_in *)u->Address.lpSockaddr)->sin_addr.s_addr;
    }
    HeapFree(GetProcessHeap(), 0, list);
    return n;
}

static DWORD WINAPI query_thread(void *arg)
{
    struct query *q = arg;
    WSADATA wsa;
    SOCKET s = INVALID_SOCKET;
    WSAEVENT ev = WSA_INVALID_EVENT;
    struct sockaddr_in sa = { AF_INET };
    BYTE pkt[9000], query[512];
    int qlen, one = 1, ttl = 255, legacy = 0;
    DWORD interval = 1000, next = GetTickCount();
    USHORT id = 0;

    if (WSAStartup(MAKEWORD(2, 2), &wsa)) goto out;

    /* full mDNS on 5353 if we can share it, else legacy unicast from an ephemeral port */
    for (int attempt = 0; attempt < 2 && s == INVALID_SOCKET; attempt++) {
        if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET) goto out;
        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&one, sizeof(one));
        sa.sin_port = attempt ? 0 : htons(MDNS_PORT);
        if (bind(s, (struct sockaddr *)&sa, sizeof(sa))) {
            closesocket(s);
            s = INVALID_SOCKET;
        }
        legacy = attempt;
    }
    if (s == INVALID_SOCKET) goto out;
    setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, (char *)&ttl, sizeof(ttl));
    setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&one, sizeof(one));
    if (legacy) id = (USHORT)(GetTickCount() ^ GetCurrentProcessId()) | 1;

    memset(query, 0, 12);
    query[0] = id >> 8;
    query[1] = id & 0xff;
    query[5] = 1; /* QDCOUNT */
    if ((qlen = write_name(query + 12, sizeof(query) - 16, q->name)) < 0) goto out;
    qlen += 12;
    query[qlen++] = q->type >> 8;
    query[qlen++] = q->type & 0xff;
    query[qlen++] = 0;
    query[qlen++] = 1; /* class IN */

    if ((ev = WSACreateEvent()) == WSA_INVALID_EVENT || WSAEventSelect(s, ev, FD_READ)) goto out;

    for (;;) {
        DWORD now = GetTickCount(), wait = (int)(next - now) > 0 ? next - now : 0;
        HANDLE handles[2] = { q->stop, ev };
        DWORD r = WaitForMultipleObjects(2, handles, FALSE, wait);

        if (r == WAIT_OBJECT_0) break;
        if (r == WAIT_OBJECT_0 + 1) {
            WSAResetEvent(ev);
            for (;;) {
                int n = recv(s, (char *)pkt, sizeof(pkt), 0);
                if (n <= 0) break;
                if (legacy && (pkt[0] << 8 | pkt[1]) != id) continue;
                handle_packet(q, pkt, n);
            }
            continue;
        }

        /* (re)join and query on every interface; picks up networks that came up later */
        ULONG addrs[MAX_IFACES];
        int n = get_ifaces(q, addrs);
        struct sockaddr_in dst = { AF_INET };
        dst.sin_port = htons(MDNS_PORT);
        dst.sin_addr.s_addr = htonl(MDNS_GROUP);
        for (int i = 0; i < n; i++) {
            struct ip_mreq mreq;
            mreq.imr_multiaddr.s_addr = htonl(MDNS_GROUP);
            mreq.imr_interface.s_addr = addrs[i];
            if (!legacy) setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq));
            setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, (char *)&addrs[i], sizeof(addrs[i]));
            sendto(s, (char *)query, qlen, 0, (struct sockaddr *)&dst, sizeof(dst));
        }
        next = GetTickCount() + interval;
        if (interval < RESEND_MAX_MS) interval = min(interval * 2, RESEND_MAX_MS);
    }

out:
    if (ev != WSA_INVALID_EVENT) WSACloseEvent(ev);
    if (s != INVALID_SOCKET) closesocket(s);
    WSACleanup();
    if (InterlockedCompareExchange(&q->self_free, 2, 1) == 1) {
        CloseHandle(q->stop);
        CloseHandle(q->thread);
        HeapFree(GetProcessHeap(), 0, q);
    }
    return 0;
}

__declspec(dllexport) DNS_STATUS WINAPI DnsStartMulticastQuery(PMDNS_QUERY_REQUEST req, PMDNS_QUERY_HANDLE handle)
{
    struct query *q;
    int n;

    if (!req || !handle || req->Version != DNS_QUERY_REQUEST_VERSION1 || !req->Query || !req->pQueryCallback)
        return ERROR_INVALID_PARAMETER;
    if (!pDnsRecordSetCopyEx && !(pDnsRecordSetCopyEx = (void *)GetProcAddress(real_dnsapi, "DnsRecordSetCopyEx")))
        return ERROR_CALL_NOT_IMPLEMENTED;
    if (!(q = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*q)))) return ERROR_OUTOFMEMORY;

    if (!(n = WideCharToMultiByte(CP_UTF8, 0, req->Query, -1, q->name, sizeof(q->name), NULL, NULL))) {
        HeapFree(GetProcessHeap(), 0, q);
        return ERROR_INVALID_PARAMETER;
    }
    if (n > 1 && q->name[n - 2] == '.') q->name[n - 2] = 0;

    /* the request usually lives on the caller's stack, so copy what we need */
    q->callback = req->pQueryCallback;
    q->context = req->pQueryContext;
    q->options = req->QueryOptions;
    q->ifindex = req->InterfaceIndex;
    q->type = req->QueryType;
    q->handle = handle;
    memset(handle, 0, sizeof(*handle));
    lstrcpynW(handle->nameBuf, req->Query, DNS_MAX_NAME_BUFFER_LENGTH);
    handle->wType = req->QueryType;
    handle->pSubscription = q;

    if (!(q->stop = CreateEventW(NULL, TRUE, FALSE, NULL))
        || !(q->thread = CreateThread(NULL, 0, query_thread, q, 0, &q->tid))) {
        if (q->stop) CloseHandle(q->stop);
        handle->pSubscription = NULL;
        HeapFree(GetProcessHeap(), 0, q);
        return ERROR_OUTOFMEMORY;
    }
    return ERROR_SUCCESS;
}

__declspec(dllexport) DNS_STATUS WINAPI DnsStopMulticastQuery(PMDNS_QUERY_HANDLE handle)
{
    struct query *q;

    if (!handle || !(q = handle->pSubscription)) return ERROR_INVALID_PARAMETER;
    handle->pSubscription = NULL;
    SetEvent(q->stop);

    /* callers free their context right after this returns, so no callback may still be running */
    if (GetCurrentThreadId() == q->tid) {
        InterlockedExchange(&q->self_free, 1);
        return ERROR_SUCCESS;
    }
    WaitForSingleObject(q->thread, INFINITE);
    CloseHandle(q->stop);
    CloseHandle(q->thread);
    HeapFree(GetProcessHeap(), 0, q);
    return ERROR_SUCCESS;
}

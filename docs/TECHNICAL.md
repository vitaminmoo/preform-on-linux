# How the shims work

PreForm (tested: 3.64.1.664 release, 3.65.0.1063 beta) installs fine under Proton, but
won't start without two shim DLLs. With them it runs, and finds printers on the LAN via
mDNS. Tested with Proton Hotfix and GE-Proton10-25, via umu/Lutris.

Install layout (the release goes to `PreForm\`, the beta bundle to `PreForm-beta\`;
the Burn bundle ignores `InstallFolder=`):

```
C:\Program Files\Formlabs\PreForm\
    PreFormLauncher.exe          <- picks the newest version dir and runs it
    Qt6Core.dll, msvcp140*.dll, vcruntime140*.dll
    3.64.1.664\
        PreForm.exe, Qt6*.dll, QtWebEngineProcess.exe, ...
        dnsapi.dll               <- shim #2, copied in by prelaunch.sh
C:\windows\system32\
    icuuc.dll                    <- shim #1
```

## 1. `icuuc.dll`: ICU forwarder

### Symptom

The launcher exits immediately and no window ever appears. With `WINEDEBUG=err`:

```
err:module:import_dll Library icuuc.dll (which is needed by L"...\PreForm\Qt6Core.dll") not found
err:module:import_dll Library Qt6Core.dll (which is needed by L"...\PreFormLauncher.exe") not found
err:module:loader_init Importing dlls for L"...\PreFormLauncher.exe" failed, status c0000135
```

### Cause

Both copies of `Qt6Core.dll` (the launcher's and the one in the version dir) were built
against the ICU that ships with Windows 10 1903+. Windows provides that ICU as `icu.dll`
plus `icuuc.dll`/`icuin.dll` forwarders, all exporting **unversioned** names
(`u_strToUpper`, `ucnv_open`, ...). Proton ships a builtin `icu.dll` that exports the
same unversioned names (Valve's `dlls/icu`, forwarding to official ICU binaries), plus
`icuuc68.dll` (versioned names, so no use here). It does not ship `icuuc.dll`. Upstream
Wine ships no ICU at all, which is why this needs a Proton build.

Qt6Core imports 22 symbols from `icuuc.dll`. All 22 are exported by Proton's `icu.dll`.

### Fix

`shims/icuuc` is a data-only PE DLL whose export table forwards every `icu.dll` export
(1031 of them) to `icu.<name>`, the same way the real Windows `icuuc.dll` works. Copying
`icu.dll` to `icuuc.dll` doesn't work: it's a Wine builtin (`Wine builtin DLL` marker),
so under a different name the loader would look for a nonexistent builtin `icuuc`.

It goes in `system32` (64-bit only, because PreForm is x64-only). Proton doesn't manage
that filename, so prefix updates leave it alone.

`make regen PROTON=...` regenerates `icuuc.def` from a Proton's `icu.dll`. The export
list is identical in Proton Hotfix, Proton Experimental and GE-Proton10-25.

## 2. `dnsapi.dll`: proxy that implements mDNS

### Symptom

With `icuuc.dll` in place, `PreForm.exe` loads and then dies:

```
Unhandled exception: unimplemented function DNSAPI.dll.DnsStartMulticastQuery called in 64-bit code
```

### Cause

PreForm discovers printers with `DnsStartMulticastQuery`/`DnsStopMulticastQuery`. On
Windows these hand the query to the DNS Client service, which owns UDP 5353 and calls
back with each answer. Proton's `dnsapi` doesn't export them at all, so the loader binds
the import to a placeholder that aborts when called. Upstream Wine 11.13+ exports stubs
that return success without doing anything
([MR 11307](https://gitlab.winehq.org/wine/wine/-/merge_requests/11307), filed for
PreForm), which avoids the crash but still never finds a printer.

DNSAPI imports across the PreForm binaries:

| Module | Imports |
|---|---|
| `PreForm.exe` | `DnsFree`, `DnsStartMulticastQuery`, `DnsStopMulticastQuery` |
| `WinFormPreviewer.dll` | `DnsStopMulticastQuery` |
| `Qt6Network.dll` | `DnsFree`, `DnsQueryEx`, `DnsExtractRecordsFromMessage_W`, `DnsWriteQuestionToBuffer_W` |

What PreForm does with it (`MdnsBrowser_win.cpp`, from the disassembly):

- One request: version 1, `_formlabs_formule._tcp.local`, type PTR, any interface. The
  request is on the stack; the `MDNS_QUERY_HANDLE` is a member of the browser object.
- The callback ignores everything but PTR records. It takes `Data.PTR.pNameHost`, strips
  the `._formlabs_formule._tcp.local` suffix, emits the instance name (e.g.
  `Form4-SillyName`), and frees the list with `DnsFree(..., DnsFreeRecordList)`.
- It calls `DnsStopMulticastQuery` and frees the browser object right after, so Stop must
  not return while a callback might still run.

### Fix

`shims/dnsapi` is a proxy `dnsapi.dll` placed next to `PreForm.exe` and preferred over the
builtin with `WINEDLLOVERRIDES=dnsapi=n,b`.

- `mdns.c` implements both functions as a small RFC 6762 continuous querier on Winsock,
  IPv4 only. One thread per query: it binds UDP 5353 with `SO_REUSEADDR` (shared with
  avahi-daemon, which also sets it), joins 224.0.0.251 on every up interface, and sends
  the question at 0, 1, 2, 4, ... seconds, capped at 60. If 5353 can't be shared, it
  falls back to an ephemeral port and legacy unicast responses (RFC 6762 §6.7).
- Each response that answers the question becomes a `DNS_RECORDW` list (answers first,
  then any PTR/SRV/A/AAAA/TXT it carried) passed to the callback. Records with TTL 0
  (goodbyes) are dropped. The list is built locally and then copied with the real
  dnsapi's `DnsRecordSetCopyEx`, so the caller can free it with Wine's `DnsFree`.
- `DnsStopMulticastQuery` signals the thread and waits for it, unless it's called from
  inside the callback, in which case the thread cleans up after itself.
- Every other export of Wine's `dnsapi` (131 of them) is an asm `jmp *real[i](%rip)`
  thunk (`thunks.S`). `DllMain` fills `real[]` with `LoadLibraryW(<system32>\dnsapi.dll)`
  and `GetProcAddress`. Loading the system copy by full path gives us the builtin, not
  ourselves, because the module path differs. Export forwarders (`X = dnsapi.X`) wouldn't
  work here, because they'd resolve back to the proxy.

`thunks.S`, `dnsapi.def` and `exports.h` are generated by `tools/gen-dnsapi.py` from a
Proton's `dnsapi.dll` (`make regen`). The export list matches across Proton Hotfix,
Experimental and GE-Proton10-25.

### Testing without a printer

```sh
avahi-publish -s Form4-FakeTest _formlabs_formule._tcp 35 "machine=Form 4"
```

PreForm's log (`%LOCALAPPDATA%\Formlabs\PreForm\Logs\log.txt`) then shows
`MdnsBrowser::Impl: service added: "Form4-FakeTest"`. Without the working shim it shows
`DnsStartMulticastQuery failed with status 120`.

### Caveats

- **Placement:** the proxy has to sit next to `PreForm.exe` in the version dir. PreForm
  updates itself into a new version dir, so the Lutris installer writes `prelaunch.sh`,
  which copies it into every version dir before each launch. It can't go in `system32`:
  that's where the real dnsapi lives, and Proton rewrites builtin DLLs there on prefix
  updates.
- **IPv4 only.** Formlabs printers advertise IPv4, so this hasn't mattered.
- **Firewall:** responses arrive on UDP 5353 (or on the ephemeral port in fallback mode).
  Fedora's default zones allow mDNS; a stricter firewall needs it opened.
- **Thunk table:** fixed at build time. If a future Proton adds `dnsapi` exports that
  PreForm or Qt start importing, run `make regen` and rebuild.

## Other launch notes

- `QTWEBENGINE_CHROMIUM_FLAGS=--no-sandbox`: Chromium's sandbox doesn't work under Wine.
- `QtWebEngineProcess` (V8) reserves about 32 GiB of address space. Under an address-space
  limit (`ulimit -v`) it fails with
  `err:virtual:allocate_virtual_memory out of memory ... size 800000000` and hits a
  breakpoint (0x80000003). Desktop sessions normally have no limit.
- Manual launch for debugging (`systemd-run --user` escapes a shell's `ulimit -v`):

  ```sh
  systemd-run --user -E WINEDLLOVERRIDES='dnsapi=n,b' -E WINEPREFIX=$HOME/Games/preform \
    -E PROTONPATH=$HOME/.local/share/lutris/runners/wine/ge-proton \
    -E GAMEID=umu-default -E STORE=none -E QTWEBENGINE_CHROMIUM_FLAGS=--no-sandbox \
    -E WINEDEBUG=+loaddll,err -E DISPLAY=$DISPLAY -E WAYLAND_DISPLAY=$WAYLAND_DISPLAY \
    ~/.local/share/lutris/runtime/umu/umu-run \
    "$HOME/Games/preform/drive_c/Program Files/Formlabs/PreForm/PreFormLauncher.exe"
  ```

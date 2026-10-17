# Getting to zero shims

Both shims cover gaps in Proton, not bugs in PreForm. Each gap can be closed upstream,
and some of them from Formlabs' side with a small change.

## In Wine / Proton

| Gap | Where | Status |
|---|---|---|
| No `icuuc.dll`/`icuin.dll` | Proton (Valve's `wine/dlls/icu` already provides `icu.dll`) | Not filed. The fix is two forwarder DLLs next to `icu.dll`, as on Windows. |
| `DnsStartMulticastQuery`/`DnsStopMulticastQuery` missing | Wine `dlls/dnsapi` | Wine 11.13+ has stubs that return success ([MR 11307](https://gitlab.winehq.org/wine/wine/-/merge_requests/11307), filed for PreForm). Proton 11 doesn't have them yet. `shims/dnsapi/mdns.c` is a working implementation that could replace those stubs. |
| Chromium sandbox | Qt WebEngine under Wine | Known; `--no-sandbox` is the standard workaround. |

Once Proton ships `icuuc.dll` and a working `DnsStartMulticastQuery`, PreForm runs on
stock Proton with only the `--no-sandbox` environment variable.

## On Formlabs' side

Each of these is small, and would make PreForm run on stock Proton today:

1. **ICU:** link Qt against the Windows SDK's combined `icu.lib` instead of
   `icuuc.lib`/`icuin.lib`, so Qt imports `icu.dll`. That's the same system ICU on
   Windows 10 1903+, and Proton already ships `icu.dll`. Bundling ICU with Qt also works.
2. **mDNS:** PreForm already has a non-Windows mDNS browser (the Windows one is
   `MdnsBrowser_win.cpp`). Using a portable implementation on Windows too, or falling
   back to one when `DnsStartMulticastQuery` fails, removes the dependency on the
   Windows DNS Client service.
3. **WebEngine:** pass `--no-sandbox` when running under Wine (`wine_get_version` is
   exported from `ntdll.dll`).

With those, Formlabs could distribute PreForm on Steam and let Proton handle Linux and
the Steam Deck, with no Linux-specific build to maintain. A native build may be closer
than it looks, too: PreForm is Qt 6, and Formlabs already ships the headless
PreFormServer for Linux.

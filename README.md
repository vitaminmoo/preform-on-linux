# PreForm on Linux

How to run [Formlabs PreForm](https://formlabs.com/software/preform/) on Linux, plus
the small shim DLLs that make it work.

PreForm is Windows/macOS only, but its Windows build runs under
[Proton](https://github.com/ValveSoftware/Proton) once two gaps in Proton are filled:

| Shim | Without it | What it does |
|---|---|---|
| `icuuc.dll` | Launcher exits silently: `Qt6Core.dll` can't find `icuuc.dll` | Forwards to the `icu.dll` Proton already ships, the way Windows' own `icuuc.dll` does |
| `dnsapi.dll` | Crash at startup: `unimplemented function DNSAPI.dll.DnsStartMulticastQuery` | Proxies Wine's `dnsapi` and implements mDNS, so printers on your network show up |

PreForm itself is unmodified and comes straight from formlabs.com. Prebuilt shims are on
the [releases page](https://github.com/vitaminmoo/preform-on-linux/releases), built
by CI from the source in [`shims/`](shims).

## Status

Tested on Fedora 44 with PreForm 3.64.1 (release) and 3.65.0 (beta), on GE-Proton 10
and Proton Hotfix:

- Works: startup, sign-in, Dashboard, opening and slicing models.
- Works: printer discovery over mDNS, tested against a printer advertised with
  `avahi-publish`.
- Not tested yet: connecting to and printing on real networked printers, and USB.
  Reports welcome.

## Install with Lutris (recommended)

1. Install [Lutris](https://lutris.net) 0.5.17 or newer.
2. Download [`lutris/preform.yaml`](lutris/preform.yaml).
3. Run `lutris -i preform.yaml`, or in Lutris click **+** → *Install from a local install
   script*, and accept the defaults.

Lutris then:

- uses GE-Proton (Latest),
- downloads PreForm from formlabs.com and runs its installer silently,
- puts `icuuc.dll` in the prefix's `system32`,
- sets the DLL override `dnsapi=n,b` and `QTWEBENGINE_CHROMIUM_FLAGS=--no-sandbox`,
- adds a pre-launch script that copies `dnsapi.dll` next to every installed
  `PreForm.exe`, so it survives PreForm's self-updates.

## Install without Lutris (umu)

Uses [umu-launcher](https://github.com/Open-Wine-Components/umu-launcher), which runs
Proton outside Steam. Any Proton build works except plain Wine and Wine-GE, which have no
`icu.dll`.

```sh
export WINEPREFIX=~/Games/preform GAMEID=umu-default PROTONPATH=GE-Proton
rel=https://github.com/vitaminmoo/preform-on-linux/releases/latest/download

# 1. Install PreForm
curl -LO https://downloads.formlabs.com/PreForm/Release/preform.exe
umu-run preform.exe /quiet /norestart

# 2. Add the shims
curl -L -o "$WINEPREFIX/drive_c/windows/system32/icuuc.dll" "$rel/icuuc.dll"
for d in "$WINEPREFIX/drive_c/Program Files/Formlabs/PreForm"/*/; do
    [ -f "$d/PreForm.exe" ] && curl -L -o "$d/dnsapi.dll" "$rel/dnsapi.dll"
done

# 3. Run
WINEDLLOVERRIDES=dnsapi=n,b QTWEBENGINE_CHROMIUM_FLAGS=--no-sandbox \
    umu-run "$WINEPREFIX/drive_c/Program Files/Formlabs/PreForm/PreFormLauncher.exe"
```

After PreForm updates itself, repeat the `dnsapi.dll` loop in step 2 for the new
version folder.

## Troubleshooting

PreForm's log is at
`<prefix>/drive_c/users/steamuser/AppData/Local/Formlabs/PreForm/Logs/log.txt`.

| Symptom | Cause |
|---|---|
| Launcher exits immediately, no window | `icuuc.dll` missing from `system32`, or not running under Proton. Run with `WINEDEBUG=err` to see `icuuc.dll ... not found`. |
| Crash: `unimplemented function DNSAPI.dll.DnsStartMulticastQuery` | `dnsapi.dll` isn't next to the `PreForm.exe` being run (often after an update), or the `dnsapi=n,b` override is missing. |
| Log says `DnsStartMulticastQuery failed with status 120` | An old copy of the shim that only stubs discovery. Update `dnsapi.dll`. |
| No printers found | Check the printer is visible to Linux: `avahi-browse -rt _formlabs_formule._tcp`. If it is, check your firewall allows UDP 5353 (mDNS). |
| `out of memory ... size 800000000`, then a breakpoint | An address-space limit (`ulimit -v`). Qt WebEngine reserves about 32 GiB; run without the limit. |
| Blank or crashing web views | `QTWEBENGINE_CHROMIUM_FLAGS=--no-sandbox` not set. |

## What's in this repo

| Path | |
|---|---|
| [`lutris/preform.yaml`](lutris/preform.yaml) | Lutris installer |
| [`shims/icuuc`](shims/icuuc) | `icuuc.dll`: export forwarders to Proton's `icu.dll` |
| [`shims/dnsapi`](shims/dnsapi) | `dnsapi.dll`: proxy for Wine's `dnsapi` with an mDNS querier (`mdns.c`) |
| [`tools/`](tools) | Regenerate the shims' export lists from a Proton install |
| [`docs/TECHNICAL.md`](docs/TECHNICAL.md) | How PreForm uses these APIs, and how the shims work |
| [`docs/UPSTREAM.md`](docs/UPSTREAM.md) | What Proton, Wine and Formlabs could change so no shims are needed |

## Building the shims

Needs `x86_64-w64-mingw32-gcc` (Fedora: `mingw64-gcc`).

```sh
make            # build/icuuc.dll, build/dnsapi.dll
make regen PROTON=~/.local/share/Steam/steamapps/common/Proton\ Hotfix   # refresh export lists
```

Pushing a `v*` tag makes CI build both DLLs and attach them to a GitHub release.

## License

[MIT](LICENSE). PreForm is a trademark of Formlabs; this project isn't affiliated with
or endorsed by Formlabs.

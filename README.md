# PreForm on Linux

How to run [Formlabs PreForm](https://formlabs.com/software/preform/) on Linux, plus
the small shim DLLs that make it work.

PreForm is Windows/macOS only, but its Windows build runs under
[Proton](https://github.com/ValveSoftware/Proton) once two gaps in Proton are filled:

| Shim | Without it | What it does |
|---|---|---|
| `icuuc.dll` | Launcher exits silently: `Qt6Core.dll` can't find `icuuc.dll` | Forwards to the `icu.dll` Proton already ships, the way Windows' own `icuuc.dll` does. Only needed before GE-Proton 11, and on Valve's Proton, which still lacks it. |
| `dnsapi.dll` | Crash at startup: `unimplemented function DNSAPI.dll.DnsStartMulticastQuery` | Proxies Wine's `dnsapi` and implements mDNS, so printers on your network show up |

PreForm itself is unmodified and comes straight from formlabs.com. Prebuilt shims are on
the [releases page](https://github.com/vitaminmoo/preform-on-linux/releases), built
by CI from the source in [`shims/`](shims).

## Status

Tested on Fedora 44 with PreForm 3.64.1 (release) and 3.65.0 (beta), on GE-Proton 10,
GE-Proton 11 and Proton Hotfix:

- Works: startup, sign-in, Dashboard, opening and slicing models.
- Works: printer discovery over mDNS, tested against a printer advertised with
  `avahi-publish`.
- Not tested yet: connecting to and printing on real networked printers, and USB.
  Reports welcome.

## Install

```sh
curl -fsSL https://github.com/vitaminmoo/preform-on-linux/releases/latest/download/install.sh | bash
```

That's it: PreForm shows up in your application menu, and as a `preform` command. For
the [public beta](https://downloads.formlabs.com/PreForm/Beta/preform.exe), add
`-s -- --beta` after `bash`; it installs alongside the release as `preform-beta`.

Prefer to read it first? Download [`install.sh`](install.sh) and run
`bash install.sh [--beta]`.

What it does:

- Uses [umu-launcher](https://github.com/Open-Wine-Components/umu-launcher), which runs
  Proton outside Steam; if `umu-run` isn't installed, it downloads umu's standalone
  release. umu fetches the latest GE-Proton on first run (set `PROTONPATH` to use
  another Proton).
- Downloads PreForm from formlabs.com and runs its installer silently, in its own
  prefix under `~/.local/share/preform-on-linux/`.
- Adds the shims, a launcher that keeps `dnsapi.dll` in place across PreForm's
  self-updates, and a menu entry with PreForm's own icon (extracted from your copy;
  no Formlabs artwork is redistributed here).

Running it again refreshes the shims, launcher and menu entry without reinstalling
PreForm. To remove everything, including PreForm's settings and sign-in:

```sh
curl -fsSL https://github.com/vitaminmoo/preform-on-linux/releases/latest/download/install.sh | bash -s -- --uninstall
```

### Steam and Steam Deck

After installing, you can add PreForm to Steam with **Add a Non-Steam Game** → Browse →
`~/.local/share/preform-on-linux/bin/preform`. Leave Proton/compatibility *off* for it:
the launcher runs Proton itself. (Not tested yet.)

## Install with Lutris

If you already use [Lutris](https://lutris.net) (0.5.17 or newer), there are installer
scripts for it too. Lutris downloads them from this repo's latest release:

```sh
lutris -i https://github.com/vitaminmoo/preform-on-linux/releases/latest/download/preform.yaml
lutris -i https://github.com/vitaminmoo/preform-on-linux/releases/latest/download/preform-beta.yaml
```

With Flatpak Lutris, use `flatpak run net.lutris.Lutris -i <url>`. Release and beta
both default to `~/Games/preform`; to install both, choose a different folder for the
second one. The installers set up the same shims, plus a pre-launch script for
`dnsapi.dll` and Lutris icon, banner and cover art
([`preform-art.py`](lutris/preform-art.py)). They live here because lutris.net only
accepts games.

They use Lutris's default Wine version, GE-Proton (Latest). If you've changed Lutris's
global Wine version to a non-Proton build such as Wine-GE, switch PreForm to a Proton
build in its *Configure → Runner options*.

## Install by hand

The same steps as `install.sh`, with [umu-launcher](https://github.com/Open-Wine-Components/umu-launcher):

```sh
export WINEPREFIX=~/Games/preform GAMEID=umu-preform PROTONPATH=GE-Proton
rel=https://github.com/vitaminmoo/preform-on-linux/releases/latest/download
app="$WINEPREFIX/drive_c/Program Files/Formlabs/PreForm"

# 1. Install PreForm
curl -LO https://downloads.formlabs.com/PreForm/Release/preform.exe
umu-run preform.exe /quiet /norestart

# 2. Add the shims (GE-Proton 11+ already has icuuc.dll)
sys32="$WINEPREFIX/drive_c/windows/system32"
[ -e "$sys32/icuuc.dll" ] || curl -L -o "$sys32/icuuc.dll" "$rel/icuuc.dll"
for d in "$app"/*/; do
    [ -f "$d/PreForm.exe" ] && curl -L -o "$d/dnsapi.dll" "$rel/dnsapi.dll"
done

# 3. Run
WINEDLLOVERRIDES=dnsapi=n,b QTWEBENGINE_CHROMIUM_FLAGS=--no-sandbox \
    umu-run "$app/PreFormLauncher.exe"
```

After PreForm updates itself, repeat the `dnsapi.dll` loop for the new version folder.
For the beta, download `https://downloads.formlabs.com/PreForm/Beta/preform.exe` and
use `Formlabs/PreForm-beta` in the paths.

## Troubleshooting

PreForm's log is at
`<prefix>/drive_c/users/steamuser/AppData/Local/Formlabs/PreForm/Logs/log.txt`
(the prefix is `~/.local/share/preform-on-linux/release` or `.../beta` with
`install.sh`).

| Symptom | Cause |
|---|---|
| Launcher exits immediately, no window | No `icuuc.dll` in `system32`, or not running under Proton. Run with `WINEDEBUG=err` to see `icuuc.dll ... not found`. |
| Crash: `unimplemented function DNSAPI.dll.DnsStartMulticastQuery` | `dnsapi.dll` isn't next to the `PreForm.exe` being run (often after an update), or the `dnsapi=n,b` override is missing. |
| Log says `DnsStartMulticastQuery failed with status 120` | An old copy of the shim that only stubs discovery. Update `dnsapi.dll`. |
| No printers found | Check the printer is visible to Linux: `avahi-browse -rt _formlabs_formule._tcp`. If it is, check your firewall allows UDP 5353 (mDNS). |
| `out of memory ... size 800000000`, then a breakpoint | An address-space limit (`ulimit -v`). Qt WebEngine reserves about 32 GiB; run without the limit. |
| Blank or crashing web views | `QTWEBENGINE_CHROMIUM_FLAGS=--no-sandbox` not set. |

## What's in this repo

| Path | |
|---|---|
| [`install.sh`](install.sh) | The installer above |
| [`lutris/`](lutris) | Lutris installers for the release and the beta, and the art generator |
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

To try `install.sh` with local builds, put `icuuc.dll`, `dnsapi.dll` and
`preform-art.py` in one folder and run `PREFORM_SHIMS_DIR=<folder> bash install.sh`.

Pushing a `v*` tag makes CI build the DLLs and attach them, with `install.sh`, the Lutris
installers and `preform-art.py`, to a GitHub release.

## License

[MIT](LICENSE). PreForm is a trademark of Formlabs; this project isn't affiliated with
or endorsed by Formlabs.

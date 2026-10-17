CC = x86_64-w64-mingw32-gcc
CFLAGS = -O2 -Wall
OUT = build

all: $(OUT)/icuuc.dll $(OUT)/dnsapi.dll

# Data-only DLL: every export forwards to Proton's icu.dll, no imports
$(OUT)/icuuc.dll: shims/icuuc/stub.c shims/icuuc/icuuc.def | $(OUT)
	$(CC) -shared -nostdlib -e DllMain -o $@ $^

$(OUT)/dnsapi.dll: shims/dnsapi/proxy.c shims/dnsapi/mdns.c shims/dnsapi/thunks.S shims/dnsapi/dnsapi.def shims/dnsapi/exports.h | $(OUT)
	$(CC) $(CFLAGS) -shared -static-libgcc -o $@ $(filter-out %.h,$^) -lws2_32 -liphlpapi

$(OUT):
	mkdir -p $@

# Regenerate export lists after a Proton update: make regen PROTON=~/.steam/steam/steamapps/common/Proton\ Hotfix
regen:
	@test -n "$(PROTON)" || { echo "set PROTON=<path to a Proton install>"; exit 1; }
	python3 -I tools/gen-dnsapi.py "$(PROTON)/files/lib/wine/x86_64-windows/dnsapi.dll"
	tools/gen-icuuc.sh "$(PROTON)/files/lib/wine/x86_64-windows/icu.dll"

clean:
	rm -rf $(OUT)

.PHONY: all regen clean

#!/usr/bin/env bash
# Build a development image. Successful linking does not imply playable status.
set -euo pipefail

root=$(dirname "$(dirname "$(realpath "${BASH_SOURCE[0]}")")")
export WINEPREFIX=${WINEPREFIX:-"$HOME/.wine32"}
export WINEDEBUG=${WINEDEBUG:--all}
sdk=${RV360_XDK_ROOT:-"$WINEPREFIX/drive_c/Program Files/Microsoft Xbox 360 SDK"}
export RV360_XDK_ROOT="$sdk"
if [[ ${RV360_DEBUG:-0} != 0 ]]; then
    printf 'Full image build currently supports release libraries only.\n' >&2
    exit 2
fi
for tool in link.exe imagexex.exe; do
    if [[ ! -f "$sdk/bin/win32/$tool" ]]; then
        printf 'Missing XDK tool: %s\n' "$tool" >&2
        exit 2
    fi
done

parent=${RV360_BUILD_ROOT:-"$root/build"}
mkdir -p "$parent"
parent=$(realpath "$parent")
build=$(mktemp -d "$parent/xenon.XXXXXX")
printf 'Development build directory: %s\n' "$build"
export RV360_OBJECT_DIR="$build/objects"
# One invocation, one response file; no globs over older build directories.
bash "$root/rv360/check_xenon.sh" "$root"/rvsource/Xbox/Src/*.cpp \
    "$root/rv360/audio360_backend.cpp" "$root/rv360/dx360_backend.cpp" \
    "$root/rv360/network360_stub.cpp" "$root/rv360/offline_ui_link_stubs.cpp"

objects=$(winepath -w "$RV360_OBJECT_DIR/objects.rsp")
pe=$(winepath -w "$build/revolt.exe")
xex=$(winepath -w "$build/default.xex")
map=$(winepath -w "$build/revolt.map")
libs=$(winepath -w "$sdk/lib/xbox")
if ! wine "$sdk/bin/win32/link.exe" "/NOLOGO" "@$objects" \
    "/OUT:$pe" "/MAP:$map" /MACHINE:PPCBE /SUBSYSTEM:XBOX /INCREMENTAL:NO \
    "/LIBPATH:$libs" d3d9.lib d3dx9.lib xgraphics.lib xaudio2.lib xmcore.lib \
    xapilib.lib xboxkrnl.lib xonline.lib libcmt.lib > "$build/link.log" 2>&1; then
    printf 'Link failed; see %s/link.log\n' "$build" >&2
    exit 1
fi
if [[ ! -s "$build/revolt.exe" ]]; then
    printf 'Linker reported success without producing an image.\n' >&2
    exit 1
fi
printf 'Linked: %s/revolt.exe\n' "$build"
if ! wine "$sdk/bin/win32/imagexex.exe" "/IN:$pe" "/OUT:$xex" \
    > "$build/imagexex.log" 2>&1; then
    printf 'XEX packaging failed; see %s/imagexex.log\n' "$build" >&2
    exit 1
fi
if [[ ! -s "$build/default.xex" ]]; then
    printf 'Image tool reported success without producing an XEX.\n' >&2
    exit 1
fi
staging="$root/build/rgh-current/Revolt"
mkdir -p "$staging/rv360/shaders"
cp "$build/default.xex" "$staging/default.xex"
cp "$root/rv360/shaders/"*.xvu "$staging/rv360/shaders/"
mkdir -p "$build/package/rv360/shaders"
cp "$root/rv360/shaders/transformed_vs.xvu" "$build/package/rv360/shaders/"
cp "$root/rv360/shaders/transformed_ps.xvu" "$build/package/rv360/shaders/"
cp "$root/rv360/shaders/transformed2_ps.xvu" "$build/package/rv360/shaders/"
wine "$sdk/bin/win32/imagexex.exe" /DUMP "$xex" > "$build/xex-info.txt" 2>&1
printf 'Updated RGH folder: %s\n' "$staging"
printf 'Packaged development image (not runtime-validated): %s/default.xex\n' "$build"

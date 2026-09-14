#!/usr/bin/env bash
# Compile checks only: this does not link or produce a playable XEX.
set -euo pipefail

root=$(dirname "$(dirname "$(realpath "${BASH_SOURCE[0]}")")")
export WINEPREFIX=${WINEPREFIX:-"$HOME/.wine32"}
export WINEDEBUG=${WINEDEBUG:--all}
sdk=${RV360_XDK_ROOT:-"$WINEPREFIX/drive_c/Program Files/Microsoft Xbox 360 SDK"}
compiler="$sdk/bin/win32/cl.exe"
if [[ ! -f "$compiler" || ! -f "$sdk/include/xbox/xtl.h" ]]; then
    printf 'Missing Xenon compiler/headers under %s\n' "$sdk" >&2
    exit 2
fi
command -v wine >/dev/null
command -v winepath >/dev/null

if [[ -n ${RV360_OBJECT_DIR:-} ]]; then
    out=$RV360_OBJECT_DIR
    # Refuse reuse: stale or duplicate objects must never enter a link.
    mkdir "$out"
else
    out=$(mktemp -d "${TMPDIR:-/tmp}/revolt-xenon.XXXXXX")
fi
printf 'Compile logs and objects: %s\n' "$out"
flags=(/nologo /c /W3 /X /D_XBOX /D_M_PPC /D_XBOX360)
if [[ ${RV360_DEBUG:-0} == 1 ]]; then
    flags+=(/D_DEBUG /Od)
else
    flags+=(/DNDEBUG)
fi
for include in "$sdk/include/xbox" "$sdk/include/xbox/sys" "$sdk/include/win32" "$root/rvsource/Xbox/Src"; do
    if [[ -d "$include" ]]; then
        win_include=$(winepath -w "$include")
        flags+=("/I\"$win_include\"")
    fi
done

if [[ $# == 0 ]]; then
    set -- rv360/xdk_probe.cpp rv360/dx360_backend.cpp rv360/audio360_backend.cpp \
        rvsource/Xbox/Src/MusicManager.cpp rvsource/Xbox/Src/VoiceManager.cpp \
        rvsource/Xbox/Src/VoiceCommunicator.cpp rvsource/Xbox/Src/net_xonline.cpp
fi
failed=0
number=0
objects=()
jobs=${RV360_JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || printf 2)}
cache=${RV360_CACHE_DIR:-"$root/build/.xenon-cache"}
mkdir -p "$cache"
compile_one()
{
    local index=$1 source=$2 source_win object object_win log key cache_object
    source_win=$(winepath -w "$source")
    object="$out/$index.obj"
    object_win=$(winepath -w "$object")
    log="$out/$index-$(basename "$source").log"
    key=$( {
        sha256sum "$source"
        printf '%s\n' "${flags[*]}"
        sha256sum "$root"/rvsource/Xbox/Src/*.h "$root"/rv360/*.h 2>/dev/null
    } | sha256sum | cut -d' ' -f1 )
    cache_object="$cache/$key.obj"
    if [[ -s "$cache_object" ]]; then
        cp "$cache_object" "$object"
        printf 'CACHE %s\n' "${source#"$root/"}" > "$log"
        cp "$log" "$log.result"
        return 0
    fi
    if wine "$compiler" "${flags[@]}" "$source_win" "/Fo$object_win" >"$log" 2>&1 && [[ -s "$object" ]]; then
        cp "$object" "$cache_object"
        printf 'PASS %s\n' "${source#"$root/"}" > "$log.result"
        return 0
    fi
    printf 'FAIL %s (see %s)\n' "${source#"$root/"}" "$log" > "$log.result"
    return 1
}
pids=()
result_logs=()
for source in "$@"; do
    number=$((number + 1))
    if [[ "$source" != /* ]]; then source="$root/$source"; fi
    if [[ ! -f "$source" ]]; then
        printf 'Missing source: %s\n' "$source" >&2
        failed=$((failed + 1))
        continue
    fi
    while (( ${#pids[@]} >= jobs )); do
        if ! wait "${pids[0]}"; then failed=$((failed + 1)); fi
        unset 'pids[0]'; pids=("${pids[@]}")
    done
    compile_one "$number" "$source" &
    pids+=("$!")
    result_logs+=("$out/$number-$(basename "$source").log.result")
done
for pid in "${pids[@]}"; do
    if ! wait "$pid"; then failed=$((failed + 1)); fi
done
for result in "${result_logs[@]}"; do
    [[ -f "$result" ]] || continue
    cat "$result"
    if grep -q '^PASS\|^CACHE' "$result"; then
        index=${result##*/}; index=${index%%-*}
        object="$out/$index.obj"
        objects+=("\"$(winepath -w "$object")\"")
    fi
done
printf '%s checked; %s failed.\n' "$number" "$failed"
if [[ $failed != 0 ]]; then exit 1; fi
printf '%s\n' "${objects[@]}" > "$out/objects.rsp"

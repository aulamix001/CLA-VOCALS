#!/usr/bin/env bash
# Reproducible setup for this Debian 13 cloud machine; no root access required.
set -euo pipefail
cd /workspace/CLA-VOCALS
task_deps=/workspace/deps
mkdir -p "$task_deps/apt/lists/partial" "$task_deps/apt/cache/archives/partial" "$task_deps/debs"
if [[ ! -x "$task_deps/python-tools/cmake/data/bin/cmake" ]]; then
    python3 -m pip install --target "$task_deps/python-tools" 'cmake==4.4.5'
fi
task_cmake="$task_deps/python-tools/cmake/data/bin/cmake"
task_ctest="$task_deps/python-tools/cmake/data/bin/ctest"
if [[ ! -d "$task_deps/JUCE/.git" ]]; then
    git clone --depth 1 --branch 8.0.6 https://github.com/juce-framework/JUCE.git "$task_deps/JUCE"
fi
test "$(git -C "$task_deps/JUCE" rev-parse HEAD)" = 51a8a6d7aeae7326956d747737ccf1575e61e209
task_apt=(-o "Dir::State::lists=$task_deps/apt/lists" -o "Dir::Cache=$task_deps/apt/cache"
          -o "Dir::Etc::sourcelist=$task_deps/apt/sources.list" -o Dir::Etc::sourceparts=-)
task_native_packages=(libasound2-dev libfreetype-dev libfontconfig-dev libx11-dev libxcomposite-dev
    libxcursor-dev libxext-dev libxinerama-dev libxrandr-dev libxrender-dev libxfixes-dev libxft-dev
    libxau-dev libxdmcp-dev libxcb1-dev x11proto-dev xorg-sgml-doctools xtrans-dev libpng-dev libbz2-dev)
if [[ ! -f "$task_deps/sysroot/usr/include/X11/Xlib.h" ]]; then
    printf '%s\n' 'deb [signed-by=/usr/share/keyrings/debian-archive-keyring.gpg] https://deb.debian.org/debian trixie main' > "$task_deps/apt/sources.list"
    apt-get "${task_apt[@]}" update
    (cd "$task_deps/debs"; apt-get "${task_apt[@]}" download "${task_native_packages[@]}")
    for task_name in "${task_native_packages[@]}"; do
        for task_package in "$task_deps/debs/${task_name}"_*.deb; do
            dpkg-deb -x "$task_package" "$task_deps/sysroot"
        done
    done
fi
# Development linker symlinks reference the already installed runtime libraries.
python3 - <<'PY'
from pathlib import Path
root = Path('/workspace/deps/sysroot/usr/lib/x86_64-linux-gnu')
for link in root.glob('*.so'):
    if link.is_symlink() and not link.exists():
        runtime = Path('/usr/lib/x86_64-linux-gnu') / link.readlink().name
        if runtime.exists():
            link.unlink()
            link.symlink_to(runtime)
PY
task_native_env=("CPLUS_INCLUDE_PATH=$task_deps/sysroot/usr/include:$task_deps/sysroot/usr/include/freetype2"
    "C_INCLUDE_PATH=$task_deps/sysroot/usr/include" "LIBRARY_PATH=$task_deps/sysroot/usr/lib/x86_64-linux-gnu"
    "PKG_CONFIG_SYSROOT_DIR=$task_deps/sysroot"
    "PKG_CONFIG_PATH=$task_deps/sysroot/usr/lib/x86_64-linux-gnu/pkgconfig:$task_deps/sysroot/usr/share/pkgconfig")
env "${task_native_env[@]}" "$task_cmake" -S . -B build/linux -G 'Unix Makefiles' \
    -DJUCE_DIR="$task_deps/JUCE" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-I$task_deps/sysroot/usr/include -I$task_deps/sysroot/usr/include/freetype2" \
    -DCMAKE_C_FLAGS="-I$task_deps/sysroot/usr/include" \
    -DCMAKE_EXE_LINKER_FLAGS="-L$task_deps/sysroot/usr/lib/x86_64-linux-gnu"
env "${task_native_env[@]}" "$task_cmake" --build build/linux --parallel 2
"$task_ctest" --test-dir build/linux --output-on-failure

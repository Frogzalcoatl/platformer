#!/usr/bin/env bash
set -e

echo "Creating compile_commands.json symlink for language server..."
OS_NAME="$(uname -s)"

if [ "${OS_NAME}" = "Darwin" ]; then
    ln -sf "build/macos/compile_commands.json" "${SCRIPT_DIR}/compile_commands.json"
else
    ln -sf "build/linux/compile_commands.json" "${SCRIPT_DIR}/compile_commands.json"
fi

# -n mean nonzero string length
if [ -n "${VCPKG_ROOT}" ]; then
    if [ -f "${VCPKG_ROOT}/vcpkg" ]; then
        echo "VCPKG_ROOT exists: ${VCPKG_ROOT}"
        echo
        echo "Done!"
        exit 0
    else
        echo "VCPKG_ROOT is defined, but vcpkg executable was not found inside it."
    fi
else
    echo "VCPKG_ROOT is not defined."
    echo
fi

SCRIPT_DIR="$(cd -- "$(dirname -- "$0")" && pwd)"
LOCAL_VCPKG_DIR="${SCRIPT_DIR}/.vcpkg"

if [ -f "${LOCAL_VCPKG_DIR}/vcpkg" ]; then
    echo "Local Vcpkg installation exists: ${LOCAL_VCPKG_DIR}"
    echo
    echo Done!
    exit 0
fi

if ! git --version >/dev/null 2>&1; then
    echo "Git is not installed."
    exit 1
fi

if [ ! -d "${LOCAL_VCPKG_DIR}" ]; then
    echo "Cloning vcpkg locally..."
    echo
    git clone --depth 1 https://github.com/microsoft/vcpkg.git "${LOCAL_VCPKG_DIR}"
    echo
fi

echo "Bootstrapping local Vcpkg..."
echo
sh "${LOCAL_VCPKG_DIR}/bootstrap-vcpkg.sh" -disableMetrics
echo

echo "Done!"
exit 0
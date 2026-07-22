#!/bin/bash
#* ******************************************************************************
#*  If not stated otherwise in this file or this component's LICENSE
#*  file the following copyright and licenses apply:
#*
#*  Copyright 2026 RDK Management
#*
#*  Licensed under the Apache License, Version 2.0 (the License);
#*  you may not use this file except in compliance with the License.
#*  You may obtain a copy of the License at
#*
#*  http://www.apache.org/licenses/LICENSE-2.0
#*
#*  Unless required by applicable law or agreed to in writing, software
#*  distributed under the License is distributed on an AS IS BASIS,
#*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#*  See the License for the specific language governing permissions and
#*  limitations under the License.
#*
#* ******************************************************************************

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
RESET='\033[0m'

declare -r TOP=`dirname $(realpath $0)`
UT_CORE_DEFAULT_VERSION="5.1.0"
export UT_CORE_VERSION="${UT_CORE_VERSION:-${UT_CORE_DEFAULT_VERSION}}"
RDK_HALIF_AIDL_VERSION="${RDK_HALIF_AIDL_VERSION:-main}"
HALIF_MODULE="bootreason"
OUT_TAR_FILE=vts-boot.tar.gz

# Define the directory where the repository will be cloned
declare -r UT_CORE_DIR="${TOP}/ut-core"
declare -r TOP_BUILD_DIR="${TOP}/build"
declare -r BUILD_INSTALL_PREFIX=${TOP_BUILD_DIR}/usr
declare -r RDK_HAL_DIR=${TOP}/rdk-halif-aidl
declare -r PROFILES_DIR=${TOP}/profiles

mkdir -p $TOP_BUILD_DIR

# Parse Target argument (e.g., Target=arm or target=linux). Default: linux.
TARGET="linux"
for arg in "$@"; do
    if [[ $arg == [Tt]arget=* ]]; then
        TARGET="${arg#*=}"
    fi
done

###############################################################################
# Cross-compile ARM binder runtime (libbinder.so, libutils.so, etc.) from
# the linux_binder_idl sources using the ARM cross-compiler.
#
# build_binder.sh intentionally does not build binder for ARM — it expects
# the linux-binder Yocto recipe to pre-stage it. For standalone ARM builds
# outside Yocto, we cross-compile it here with BUILD_HOST_AIDL=OFF so only
# the runtime libs are built (skipping the AIDL host tools that require a
# host/x86 compiler).
###############################################################################
build_arm_binder()
{
    local BINDER_REPO_DIR="${RDK_HAL_DIR}/build-tools/linux_binder_idl"
    local ARM_BINDER_BUILD="${RDK_HAL_DIR}/build/binder-arm"
    local ARM_BINDER_SDK="${RDK_HAL_DIR}/out/target"

    if [ -f "${ARM_BINDER_SDK}/.sdk_ready" ]; then
        echo "ARM Binder SDK already built at ${ARM_BINDER_SDK}"
        return 0
    fi

    # Clone linux_binder_idl directly — not via build_binder.sh.
    # build_binder.sh would try to build the full host AIDL compiler, but OE
    # prepends its own native tools (including an older m4) to PATH even when
    # CC/CXX are unset — that m4 rejects the --gnu flag bison passes and
    # breaks the AIDL parser build. A plain git clone avoids PATH entirely.
    if [ ! -d "${BINDER_REPO_DIR}/.git" ]; then
        echo "Cloning linux_binder_idl for ARM build..."
        local _binder_version
        _binder_version=$(grep -vE '^[[:space:]]*(#|$)' \
            "${RDK_HAL_DIR}/binder_sdk.version" 2>/dev/null \
            | head -1 | tr -d '[:space:]')
        _binder_version="${_binder_version:-2.0.0}"

        [ -d "${BINDER_REPO_DIR}" ] && rm -rf "${BINDER_REPO_DIR}"
        mkdir -p "${RDK_HAL_DIR}/build-tools"
        git clone "https://github.com/rdkcentral/linux_binder_idl" \
            "${BINDER_REPO_DIR}" || {
            echo "ERROR: Failed to clone linux_binder_idl."
            exit 1
        }
        ( cd "${BINDER_REPO_DIR}" && \
          git checkout "${_binder_version}" 2>/dev/null || \
          echo "Note: version ${_binder_version} not found, using default branch" )

        # Clone Android binder sources required by the cmake build
        ( cd "${BINDER_REPO_DIR}" && bash ./clone-android-binder-repo.sh ) || {
            echo "ERROR: Failed to clone Android binder sources."
            exit 1
        }
    fi

    # The OE toolchain sets CC/CXX to "compiler-path <flags>" (flags embedded).
    # cmake requires CMAKE_C_COMPILER to be just the binary path; flags go into
    # CMAKE_C_FLAGS_INIT inside a toolchain file.
    #
    # cmake 3.22+ auto-loads OEToolchainConfig.cmake even when CMAKE_TOOLCHAIN_FILE
    # is cleared. Writing our own toolchain file takes precedence over that.
    local cc_bin cxx_bin cc_flags
    cc_bin=$(echo "${CC}" | awk '{print $1}')
    cxx_bin=$(echo "${CXX}" | awk '{print $1}')
    cc_flags=$(echo "${CC}" | cut -d' ' -f2-)   # flags embedded in CC by OE

    mkdir -p "${ARM_BINDER_BUILD}"
    local TOOLCHAIN_FILE="${ARM_BINDER_BUILD}/arm-cross.cmake"
    cat > "${TOOLCHAIN_FILE}" <<TOOLCHAIN_EOF
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER   "${cc_bin}")
set(CMAKE_CXX_COMPILER "${cxx_bin}")
set(CMAKE_C_FLAGS_INIT   "${cc_flags} ${CFLAGS:-}")
set(CMAKE_CXX_FLAGS_INIT "${cc_flags} ${CXXFLAGS:-}")
set(CMAKE_EXE_LINKER_FLAGS_INIT    "${LDFLAGS:-}")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${LDFLAGS:-}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
TOOLCHAIN_EOF

    # linux_binder_idl cmake auto-enables BUILD_ENV_YOCTO when OECORE_* env
    # vars are set, which skips ALL install() rules. Temporarily clear these
    # vars during cmake configure so BUILD_ENV_YOCTO stays OFF.
    local _OECORE_NATIVE="${OECORE_NATIVE_SYSROOT:-}"
    local _OECORE_TARGET="${OECORE_TARGET_SYSROOT:-}"
    unset OECORE_NATIVE_SYSROOT OECORE_TARGET_SYSROOT

    echo "Cross-compiling ARM binder runtime (BUILD_HOST_AIDL=OFF)..."
    echo "  Compiler: ${cc_bin}"
    echo "  Flags:    ${cc_flags} ${CFLAGS:-}"
    cmake -S "${BINDER_REPO_DIR}" \
          -B "${ARM_BINDER_BUILD}" \
          -DCMAKE_INSTALL_PREFIX="${ARM_BINDER_SDK}" \
          -DCMAKE_BUILD_TYPE=Release \
          -DBUILD_HOST_AIDL=OFF \
          -DBUILD_ENV_YOCTO=OFF \
          -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}"
    local cmake_cfg_rc=$?

    # Restore OE env vars before any further steps
    [ -n "${_OECORE_NATIVE}" ] && export OECORE_NATIVE_SYSROOT="${_OECORE_NATIVE}"
    [ -n "${_OECORE_TARGET}" ] && export OECORE_TARGET_SYSROOT="${_OECORE_TARGET}"

    if [ $cmake_cfg_rc -ne 0 ]; then
        echo -e "${BOLD}${RED}ERROR: ARM binder cmake configuration failed.${RESET}"
        exit 1
    fi
    cmake --build "${ARM_BINDER_BUILD}" -j"$(nproc)" || {
        echo -e "${BOLD}${RED}ERROR: ARM binder build failed.${RESET}"
        exit 1
    }
    cmake --install "${ARM_BINDER_BUILD}" || {
        echo -e "${BOLD}${RED}ERROR: ARM binder install failed.${RESET}"
        exit 1
    }

    # Reorganise the install tree to match what build_binder.sh produces for
    # the linux target (out/target/lib/binder/, out/build/include/binder_sdk/).
    # Always wipe lib/binder/ — Step 1 (x86 build) may have put x86_64 libs here.
    local ARM_BINDER_INCLUDE="${RDK_HAL_DIR}/out/build"
    if [ -d "${ARM_BINDER_SDK}/lib" ]; then
        rm -rf "${ARM_BINDER_SDK}/lib/binder"
        mkdir -p "${ARM_BINDER_SDK}/lib/binder"
        mv "${ARM_BINDER_SDK}"/lib/*.so* "${ARM_BINDER_SDK}/lib/binder/" 2>/dev/null || true
    fi
    if [ -d "${ARM_BINDER_SDK}/include" ]; then
        mkdir -p "${ARM_BINDER_INCLUDE}/include/binder_sdk"
        find "${ARM_BINDER_SDK}/include" -mindepth 1 -maxdepth 1 -type d \
            -exec mv {} "${ARM_BINDER_INCLUDE}/include/binder_sdk/" \; 2>/dev/null || true
        find "${ARM_BINDER_SDK}/include" -maxdepth 1 -type f -name "*.h" \
            -exec mv {} "${ARM_BINDER_INCLUDE}/include/binder_sdk/" \; 2>/dev/null || true
        rmdir "${ARM_BINDER_SDK}/include" 2>/dev/null || true
    fi
    mkdir -p "${ARM_BINDER_SDK}/lib/halif"
    touch "${ARM_BINDER_SDK}/.sdk_ready"
    touch "${ARM_BINDER_INCLUDE}/.sdk_ready"
    echo "ARM Binder SDK built and installed to ${ARM_BINDER_SDK}"
}

###############################################################################
# Set up the Binder SDK required by build_modules.sh.
#
# Linux: runs build_binder.sh (no-op if .sdk_ready already exists).
# ARM:   uses BINDER_SDK_DIR if already set, or the OE sysroot binder if
#        available. Otherwise does two steps:
#          Step 1 — build the AIDL host tools with the x86 compiler.
#          Step 2 — cross-compile the ARM binder runtime via build_arm_binder().
#
# Sets BINDER_SDK_DIR in the environment for downstream use by build_modules.sh.
###############################################################################
setup_binder_sdk()
{
    local binder_toolchain_dir="${RDK_HAL_DIR}/build-tools/linux_binder_idl"

    if [ "${TARGET}" = "arm" ]; then
        if [ -n "${BINDER_SDK_DIR:-}" ]; then
            return
        fi

        if [ -n "${OECORE_TARGET_SYSROOT:-}" ] && [ -d "${OECORE_TARGET_SYSROOT}/usr/lib/binder" ]; then
            export BINDER_SDK_DIR="${OECORE_TARGET_SYSROOT}/usr"
            echo "ARM build: using OE sysroot binder SDK at ${BINDER_SDK_DIR}"
            return
        fi

        # Step 1: build AIDL host tools using x86 compiler.
        # Clear ARM cross-compile env AND OECORE vars — linux_binder_idl
        # CMakeLists.txt auto-sets BUILD_ENV_YOCTO=ON when OECORE_* vars
        # are present, which skips all cmake install() rules.
        echo "ARM build: building AIDL host tools with x86 compiler..."
        local _CC="${CC:-}" _CXX="${CXX:-}" _CFLAGS="${CFLAGS:-}"
        local _CXXFLAGS="${CXXFLAGS:-}" _LDFLAGS="${LDFLAGS:-}"
        local _ARCH="${ARCH:-}" _CMAKE_TOOLCHAIN_FILE="${CMAKE_TOOLCHAIN_FILE:-}"
        local _OECORE_NATIVE="${OECORE_NATIVE_SYSROOT:-}"
        local _OECORE_TARGET="${OECORE_TARGET_SYSROOT:-}"
        unset CC CXX CFLAGS CXXFLAGS LDFLAGS ARCH CMAKE_TOOLCHAIN_FILE
        unset OECORE_NATIVE_SYSROOT OECORE_TARGET_SYSROOT
        export M4=/usr/bin/m4
        (
            cd "${RDK_HAL_DIR}"
            ./build_binder.sh
        ) || {
            echo -e "${BOLD}${RED}ERROR: build_binder.sh (host tools) failed.${RESET}"
            exit 1
        }
        # Restore ARM cross-compile environment.
        [ -n "${_CC}" ]                   && export CC="${_CC}"
        [ -n "${_CXX}" ]                  && export CXX="${_CXX}"
        [ -n "${_CFLAGS}" ]               && export CFLAGS="${_CFLAGS}"
        [ -n "${_CXXFLAGS}" ]             && export CXXFLAGS="${_CXXFLAGS}"
        [ -n "${_LDFLAGS}" ]              && export LDFLAGS="${_LDFLAGS}"
        [ -n "${_ARCH}" ]                 && export ARCH="${_ARCH}"
        [ -n "${_CMAKE_TOOLCHAIN_FILE}" ] && export CMAKE_TOOLCHAIN_FILE="${_CMAKE_TOOLCHAIN_FILE}"
        [ -n "${_OECORE_NATIVE}" ]        && export OECORE_NATIVE_SYSROOT="${_OECORE_NATIVE}"
        [ -n "${_OECORE_TARGET}" ]        && export OECORE_TARGET_SYSROOT="${_OECORE_TARGET}"

        # Step 2: cross-compile ARM binder runtime.
        # Remove .sdk_ready written by Step 1 — build_arm_binder() skips
        # itself if this file exists, leaving x86_64 libs in place.
        rm -f "${RDK_HAL_DIR}/out/target/.sdk_ready"
        echo "ARM build: cross-compiling ARM binder runtime..."
        build_arm_binder
        export BINDER_SDK_DIR="${RDK_HAL_DIR}/out/target"
    else
        # Linux native build: always call build_binder.sh so linux_binder_idl
        # is cloned and host/aidl_ops.py exists before build_modules.sh preflight
        # runs.  build_binder.sh exits immediately if .sdk_ready is already
        # present, so there is no cost when the SDK is up to date.
        if [ -d "${binder_toolchain_dir}" ] && [ ! -d "${binder_toolchain_dir}/.git" ]; then
            echo "Removing incomplete toolchain stub before fresh clone..."
            rm -rf "${binder_toolchain_dir}"
        fi
        (
            cd "${RDK_HAL_DIR}"
            ./build_binder.sh
        ) || {
            echo -e "${BOLD}${RED}ERROR: build_binder.sh failed. Cannot continue.${RESET}"
            exit 1
        }
    fi
}

###############################################################################
# Install Binder service manager, headers and libraries into build/usr/ from
# the rdk-halif-aidl Binder SDK (built by setup_binder_sdk -> build_binder.sh).
# Must be called after setup_binder_sdk().
###############################################################################
install_linux_binder()
{
    local BINDER_SDK="${RDK_HAL_DIR}/out/target"
    local BINDER_INC="${RDK_HAL_DIR}/out/build"

    echo "Install binder 'service manager, headers and libraries'"
    install -d ${BUILD_INSTALL_PREFIX}/bin ${BUILD_INSTALL_PREFIX}/lib ${BUILD_INSTALL_PREFIX}/include

    if [ -d "${BINDER_SDK}/bin" ]; then
        find "${BINDER_SDK}/bin" -maxdepth 1 -type f -executable \
            -exec install -t ${BUILD_INSTALL_PREFIX}/bin {} \; 2>/dev/null || true
    fi

    if [ -d "${BINDER_SDK}/lib/binder" ]; then
        find "${BINDER_SDK}/lib/binder" -maxdepth 1 \( -name "*.so" -o -name "*.so.*" \) \
            -exec install -t ${BUILD_INSTALL_PREFIX}/lib {} \; 2>/dev/null || true
    fi

    if [ -d "${BINDER_INC}/include/binder_sdk" ]; then
        cp -apr ${BINDER_INC}/include/binder_sdk/. ${BUILD_INSTALL_PREFIX}/include/
    fi

    # Bundle host libstdc++ for linux builds only — for ARM the target system
    # provides the correct ABI-matched libstdc++ already.
    if [ "${TARGET}" = "linux" ]; then
        local libstdcpp
        libstdcpp=$(realpath "$(g++ -print-file-name=libstdc++.so.6)" 2>/dev/null)
        if [ -f "${libstdcpp}" ]; then
            install -t ${BUILD_INSTALL_PREFIX}/lib "${libstdcpp}"
            # Create the soname symlink (libstdc++.so.6) that the dynamic linker looks for
            local libstdcpp_basename
            libstdcpp_basename=$(basename "${libstdcpp}")
            ln -sf "${libstdcpp_basename}" "${BUILD_INSTALL_PREFIX}/lib/libstdc++.so.6"
            echo "Bundled host libstdc++: ${libstdcpp}"
        fi
    fi
}

###############################################################################
# Create convenience softlinks so the Makefile can link with -l<module>-cpp_vts
# without embedding the version number:
#   lib<module>-cpp_vts.so  ->  lib<module>-v<version>-cpp.so
# Versions are read from rdk-halif-aidl/versions_released.yaml.
###############################################################################
create_halif_softlinks()
{
    local YAML="${RDK_HAL_DIR}/versions_released.yaml"
    local LIB_DIR="${RDK_HAL_DIR}/out/target/lib/rdk-halif-aidl"
    local in_components=0

    if [ ! -f "$YAML" ]; then
        echo "Warning: versions_released.yaml not found, skipping softlink creation"
        return
    fi

    echo "Creating halif library softlinks..."
    while IFS= read -r line; do
        if [[ "$line" =~ ^components: ]]; then
            in_components=1
            continue
        fi
        if [[ $in_components -eq 1 ]]; then
            if [[ "$line" =~ ^[[:space:]]+([A-Za-z0-9_]+):[[:space:]]*([0-9]+\.[0-9]+\.[0-9]+\.[0-9]+) ]]; then
                local module="${BASH_REMATCH[1]}"
                local version="${BASH_REMATCH[2]}"
                local versioned="lib${module}-v${version}-cpp.so"
                local link="lib${module}-cpp_vts.so"
                if [ -f "${LIB_DIR}/${versioned}" ]; then
                    ln -sf "${versioned}" "${LIB_DIR}/${link}"
                    echo "  ${link} -> ${versioned}"
                fi
            elif [[ "$line" =~ ^[^[:space:]] && ! "$line" =~ ^# ]]; then
                in_components=0
            fi
        fi
    done < "$YAML"

    # Write halif_generated.mk so the Makefile reads exact paths captured at
    # build time rather than using wildcard/find at parse time, which silently
    # returns empty on a clean tree before build.sh has produced the files.
    local MK_FILE="${RDK_HAL_DIR}/out/halif_generated.mk"
    local mk_inc_dirs mk_module_libs
    mk_inc_dirs=$(find "${RDK_HAL_DIR}/out/build/include" -mindepth 3 -maxdepth 3 \
        -type d -name "include" 2>/dev/null | tr '\n' ' ' | sed 's/ $//')
    mk_module_libs=$(find "${LIB_DIR}" -maxdepth 1 -type l -name 'lib*-cpp_vts.so' \
        2>/dev/null | sed 's|.*/lib\(.*\)\.so|-l\1|' | tr '\n' ' ' | sed 's/ $//')
    {
        echo "# Auto-generated by build.sh create_halif_softlinks() — do not edit"
        echo "HALIF_INC_DIRS    := ${mk_inc_dirs}"
        echo "HALIF_MODULE_LIBS := ${mk_module_libs}"
    } > "${MK_FILE}"
    echo "  Generated ${MK_FILE}"
}

###############################################################################
# Clone rdk-halif-aidl if not present, or switch to the required branch.
# Invalidates the halif modules sentinel when the branch changes so the next
# build_halif_modules() call does a full rebuild.
###############################################################################
checkout_halif_repo()
{
    if [ ! -d "$RDK_HAL_DIR" ]; then
        echo "Cloning rdk-halif-aidl (branch: $RDK_HALIF_AIDL_VERSION)..."
        git clone -b "$RDK_HALIF_AIDL_VERSION" https://github.com/rdkcentral/rdk-halif-aidl.git "$RDK_HAL_DIR" || {
            echo "ERROR: Failed to clone rdk-halif-aidl."
            exit 1
        }
        return
    fi

    local current_branch
    current_branch=$(git -C "$RDK_HAL_DIR" rev-parse --abbrev-ref HEAD 2>/dev/null)
    if [ "${current_branch}" = "${RDK_HALIF_AIDL_VERSION}" ]; then
        echo "rdk-halif-aidl already on '${RDK_HALIF_AIDL_VERSION}'"
        return
    fi

    echo "rdk-halif-aidl is on '${current_branch}', switching to '${RDK_HALIF_AIDL_VERSION}'..."
    git -C "$RDK_HAL_DIR" fetch origin "$RDK_HALIF_AIDL_VERSION" || {
        echo "ERROR: Failed to fetch branch ${RDK_HALIF_AIDL_VERSION}."
        exit 1
    }
    git -C "$RDK_HAL_DIR" checkout "$RDK_HALIF_AIDL_VERSION" || {
        echo "ERROR: Failed to checkout branch ${RDK_HALIF_AIDL_VERSION}."
        exit 1
    }
    # Branch changed — invalidate the halif modules stamp so they are rebuilt.
    rm -f "${RDK_HAL_DIR}/out/target/lib/rdk-halif-aidl/.halif_modules_ready"
}

###############################################################################
# Build HAL modules from the versioned snapshot manifest (versions_released.yaml)
# into lib<module>-v<version>-cpp.so, then create unversioned softlinks.
# Skips the build entirely if the sentinel .halif_modules_ready already exists.
###############################################################################
build_halif_modules()
{
    local HALIF_LIB_DIR="${RDK_HAL_DIR}/out/target/lib/rdk-halif-aidl"
    local HALIF_READY="${HALIF_LIB_DIR}/.halif_modules_ready"

    # Resolve the module version from versions_released.yaml, falling back
    # to the default: value if the module is not explicitly listed.
    local versions_file="${RDK_HAL_DIR}/versions_released.yaml"
    local version
    version=$(grep -m1 "^[[:space:]]*${HALIF_MODULE}:" "$versions_file" | awk '{print $2}')
    if [ -z "$version" ]; then
        version=$(grep -m1 '^default:' "$versions_file" | awk '{print $2}')
        version="${version:-current}"
        echo -e "${BOLD}${YELLOW}WARNING: ${HALIF_MODULE} not listed in versions_released.yaml, using default: ${version}${RESET}"
    fi

    if [ ! -f "${HALIF_READY}" ]; then
        echo "Building ${HALIF_MODULE} module (version: ${version}) via build_modules.sh..."
        local sdk_dir_arg=""
        if [ "${TARGET}" = "arm" ] && [ -n "${BINDER_SDK_DIR:-}" ]; then
            sdk_dir_arg="--sdk-dir ${BINDER_SDK_DIR}"
        fi
        (
            cd "${RDK_HAL_DIR}"
	    if [ "${TARGET}" = "arm" ]; then
                export BINDER_ALLOW_CROSS_ENV=1
            fi
            ./build_modules.sh "${HALIF_MODULE}" --version "${version}" ${sdk_dir_arg}
        ) || {
            echo -e "${BOLD}╔══════════════════════════════════════════════════════════════════════╗${RESET}"
            echo -e "${BOLD}* ${RED}Failed to build ${HALIF_MODULE} module. Exiting.${RESET}"
            echo -e "${BOLD}╚══════════════════════════════════════════════════════════════════════╝${RESET}"
            exit 1
        }
        touch "${HALIF_READY}"
    else
        echo "HAL module .so already present, skipping build_modules.sh..."
    fi

    # build_modules.sh single-module mode does not pre-stage headers to
    # out/build/include/ (only manifest mode does this for dep resolution).
    # Stage them so create_halif_softlinks() can populate HALIF_INC_DIRS.
    # Always re-stage on every build to keep in sync with version changes.
    local inc_src="${RDK_HAL_DIR}/${HALIF_MODULE}/${version}/include"
    local inc_dst="${RDK_HAL_DIR}/out/build/include/${HALIF_MODULE}/${version}/include"
    if [ -d "${inc_src}" ]; then
        mkdir -p "${inc_dst}"
        cp -apr "${inc_src}/." "${inc_dst}/"
        echo "Staged ${HALIF_MODULE} headers: ${inc_dst}"
    else
        echo -e "${BOLD}${YELLOW}WARNING: header source not found: ${inc_src}${RESET}"
    fi

    create_halif_softlinks
}

###############################################################################
# Clone rdk-halif-aidl and build HAL interfaces using its own build system.
#
# build_modules.sh manifest:
#   - Reads versions_released.yaml and compiles each component's pre-committed
#     C++ from the pinned versioned snapshot into lib<module>-v<version>-cpp.so
#   - Outputs libraries to rdk-halif-aidl/out/target/lib/rdk-halif-aidl/
#   - Outputs headers  to rdk-halif-aidl/out/build/include/<module>/<version>/include/
#
# For ARM cross-compilation set CC/CXX/CFLAGS/CXXFLAGS/LDFLAGS in the
# environment and BINDER_SDK_DIR to the ARM-compiled Binder SDK location.
###############################################################################
generate_hal_interfaces()
{
    checkout_halif_repo
    setup_binder_sdk
    build_halif_modules
}

###############################################################################
# Function to clone and build the ut-core repository
###############################################################################
build_ut_core()
{
    if [ ! -d ${UT_CORE_DIR} ]; then
        git clone https://github.com/rdkcentral/ut-core.git ${UT_CORE_DIR}
        if [ $? -ne 0 ]; then
            echo "Failed to clone the repository. Exiting."
            exit 1
        fi
    fi

    pushd ${UT_CORE_DIR}

    git fetch -q -t origin
    git checkout $UT_CORE_VERSION

    ./build.sh TARGET=$TARGET VARIANT=CPP
    if [ $? -ne 0 ]; then
        echo -e "${BOLD}╔══════════════════════════════════════════════╗${RESET}"
        echo -e "${BOLD}* ${RED}Failed to build the UT Core. Exiting.${RESET}        ${BOLD}*${RESET}"
        echo -e "${BOLD}╚══════════════════════════════════════════════╝${RESET}"
        exit 1
    fi

    popd

}

###############################################################################
# Function to Build HALIF test
###############################################################################
build_halif_test()
{
    # Build the tests
    make -j`nproc` -C ${TOP} TARGET=$TARGET VARIANT=CPP
    if [ $? -ne 0 ]; then
        echo -e "${BOLD}╔════════════════════════════════════════════╗${RESET}"
        echo -e "${BOLD}* ${RED}Failed to build the main tests. Exiting.${RESET}"
        echo -e "${BOLD}╚════════════════════════════════════════════╝${RESET}"
        exit 1
    fi
}

###############################################################################
# Function to install test binaries and scripts to the build directory
###############################################################################
install_test()
{
    declare -r INSTALL_PATH=${TOP_BUILD_DIR}/bin

    install -t ${INSTALL_PATH} ${TOP_BUILD_DIR}/usr/bin/*
    install -t ${INSTALL_PATH} ${TOP_BUILD_DIR}/usr/lib/*
    install -t ${INSTALL_PATH} run.sh

    # Remove bundled binder runtime libs — the target's system libbinder.so is
    # compatible with its kernel binder driver. Bundling our compiled version
    # causes a kernel/userspace protocol mismatch at runtime.
    rm -f "${INSTALL_PATH}"/libbinder.so* \
          "${INSTALL_PATH}"/libutils.so* \
          "${INSTALL_PATH}"/libbase.so* \
          "${INSTALL_PATH}"/libcutils.so* \
          "${INSTALL_PATH}"/libcutils_sockets.so* \
          "${INSTALL_PATH}"/liblog.so* \
          "${INSTALL_PATH}"/servicemanager

    # Install versioned halif libs and their softlinks from the snapshot build
    local HALIF_LIB_DIR="${RDK_HAL_DIR}/out/target/lib/rdk-halif-aidl"
    if [ -d "${HALIF_LIB_DIR}" ]; then
        find "${HALIF_LIB_DIR}" -maxdepth 1 -name "*.so" \
            -exec cp -a {} "${INSTALL_PATH}/" \; 2>/dev/null || true
    fi

    cp -r ${PROFILES_DIR} ${INSTALL_PATH}
    cp -r "${TOP}/test_configuration" ${INSTALL_PATH}

    cd ${INSTALL_PATH}
    tar -czf ${OUT_TAR_FILE} *
    cd -
}

###############################################################################
# Function to clean the test build directory
###############################################################################
clean_test()
{
    echo -e "${BOLD}removing *.o${RESET}"

    rm -rf test/*.o
    rm -rf ${TOP}/src/*.o
    make clean
    rm -rf ${TOP_BUILD_DIR}/bin/${OUT_TAR_FILE}
}

###############################################################################
# Function to clean the test build directory
###############################################################################
dist_clean_test()
{
    clean_test
    rm -rf $TOP_BUILD_DIR
    rm -rf $RDK_HAL_DIR
    rm -rf ${UT_CORE_DIR}
}

###############################################################################
# help function to display usage information
###############################################################################
help()
{
    echo -e "${BOLD}${CYAN}Usage:${RESET}"

    echo -e "${BOLD}╔════════════════════════════════════════════════════════════════════════════════╗${RESET}"
    echo -e "${BOLD}* ${GREEN}$0 TARGET=<target>${RESET} ${YELLOW}- Builds the test for specified target (arm, linux)${RESET} ${BOLD}*${RESET}"
    echo -e "${BOLD}* ${GREEN}$0 clean${RESET}           ${YELLOW}- Cleans the build folder${RESET}                           ${BOLD}*${RESET}"
    echo -e "${BOLD}* ${GREEN}$0 dist_clean${RESET}      ${YELLOW}- Deletes all build and checkout folders${RESET}            ${BOLD}*${RESET}"
    echo -e "${BOLD}* ${GREEN}$0 help/-h/--help${RESET}  ${YELLOW}- Prints this message${RESET}                               ${BOLD}*${RESET}"
    echo -e "${BOLD}╚════════════════════════════════════════════════════════════════════════════════╝${RESET}"
}


echo -e "${BOLD}${GREEN}VTS Version            : ${RESET}${YELLOW}$(git rev-parse --abbrev-ref HEAD)${RESET}"
echo -e "${BOLD}${GREEN}UT_CORE_VERSION        : ${RESET}${YELLOW}$UT_CORE_VERSION${RESET}"
echo -e "${BOLD}${GREEN}RDK_HALIF_AIDL_VERSION : ${RESET}${YELLOW}$RDK_HALIF_AIDL_VERSION${RESET}"
echo -e "${BOLD}${GREEN}HALIF_MODULE           : ${RESET}${YELLOW}$HALIF_MODULE${RESET}"
echo -e "${BOLD}${GREEN}TARGET                 : ${RESET}${YELLOW}$TARGET${RESET}"

###############################################################################
# Main script execution starts here
###############################################################################
if [ "$1" == "clean" ]; then
    clean_test
elif [ "$1" == "dist_clean" ]; then
    dist_clean_test
elif [ "$1" == "help" ] || [ "$1" == "-h" ] || [ "$1" == "--help" ]; then
    help
elif [[ "$1" == [Tt]arget=* ]]; then
    # Validate TARGET
    valid_targets=("linux" "arm")
    if [[ ! " ${valid_targets[*]} " =~ " ${TARGET} " ]]; then
        echo -e "${BOLD}${RED}Invalid Target: ${TARGET}. Valid targets are: ${valid_targets[*]}${RESET}"
        help
        exit 1
    fi
    generate_hal_interfaces
    install_linux_binder
    build_ut_core
    build_halif_test
    install_test
    echo -e "${BOLD}╔════════════════════════════════╗${RESET}"
    echo -e "${BOLD}* ${GREEN}Build Successful${RESET}               ${BOLD}*${RESET}"
    echo -e "${BOLD}╚════════════════════════════════╝${RESET}"
else
    echo -e "${BOLD}${RED}Invalid Arguments${RESET}"
    help
fi

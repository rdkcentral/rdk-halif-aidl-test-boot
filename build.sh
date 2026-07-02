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
RDK_HALIF_AIDL_VERSION="${RDK_HALIF_AIDL_VERSION="main"}"
RDK_HALIF_TEST_BASE_URL="https://github.com/rdkcentral/rdk-halif-binder-test-"
OUT_TAR_FILE=vts-boot.tar.gz

declare -A HALIF_COMPONENTS=(
    [bootreason]="current"
)

# Define the directory where the repository will be cloned
declare -r UT_CORE_DIR="${TOP}/ut-core"
declare -r TOP_BUILD_DIR="${TOP}/build"
declare -r COMMON_DIR="${TOP}/common"
declare -r BUILD_INSTALL_PREFIX=${TOP_BUILD_DIR}/usr
declare -r AIDL_LIB_DIR=${TOP}/aidl_lib
declare -r RDK_HAL_DIR=${TOP}/rdk-halif-aidl
declare -r RDK_HAL_TEST_DIR=${TOP}/vts_hal_test_components
declare -r PROFILES_DIR=${TOP}/profiles
declare -r AV_L4_TEST_DIR=${RDK_HAL_TEST_DIR}/av_l4_tests

LOCAL_BINDER_TOOLCHAIN_DIR=${BINDER_SOURCE_DIR:-${RDK_HAL_DIR}/build-tools/linux_binder_idl}
LOCAL_BINDER_SDK_DIR=${BINDER_SDK_DIR:-${RDK_HAL_DIR}/out/target}
LOCAL_BINDER_INCLUDE_DIR=${BINDER_SDK_INCLUDE_DIR:-${RDK_HAL_DIR}/out/build/include/binder_sdk}
LOCAL_BINDER_LIB_DIR=${BINDER_SDK_LIB_DIR:-${LOCAL_BINDER_SDK_DIR}/lib/binder}

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
    touch "${ARM_BINDER_SDK}/.sdk_ready"
    echo "ARM Binder SDK built and installed to ${ARM_BINDER_SDK}"
}

###############################################################################
# Set up the Binder SDK required for compilation.
#
# Linux: runs rdk-halif-aidl/build_binder.sh (no-op if .sdk_ready exists).
# ARM:   checks in order —
#   1. BINDER_SDK_DIR env var (user-supplied pre-built ARM binder SDK).
#   2. OE sysroot binder at ${OECORE_TARGET_SYSROOT}/usr/lib/binder.
#   3. Two-step cross-compile:
#        Step 1 — build host AIDL tools with the x86 compiler (unsets OE env
#                 so linux_binder_idl does not skip its install() rules).
#        Step 2 — cross-compile the ARM binder runtime via build_arm_binder().
###############################################################################
setup_binder_sdk()
{
    local binder_toolchain_dir="${LOCAL_BINDER_TOOLCHAIN_DIR}"
    local sdk_ready="${LOCAL_BINDER_SDK_DIR}/.sdk_ready"

    if [ "${TARGET}" = "arm" ]; then
        # Option 1: user supplied a pre-built ARM binder SDK
        if [ -n "${BINDER_SDK_DIR:-}" ]; then
            echo "ARM build: using pre-built Binder SDK at ${BINDER_SDK_DIR}"
            LOCAL_BINDER_SDK_DIR="${BINDER_SDK_DIR}"
            LOCAL_BINDER_LIB_DIR="${BINDER_SDK_DIR}/lib/binder"
            return
        fi

        # Option 2: OE sysroot already contains the binder runtime
        if [ -n "${OECORE_TARGET_SYSROOT:-}" ] && [ -d "${OECORE_TARGET_SYSROOT}/usr/lib/binder" ]; then
            export BINDER_SDK_DIR="${OECORE_TARGET_SYSROOT}/usr"
            LOCAL_BINDER_SDK_DIR="${BINDER_SDK_DIR}"
            LOCAL_BINDER_LIB_DIR="${BINDER_SDK_DIR}/lib/binder"
            echo "ARM build: using OE sysroot binder SDK at ${BINDER_SDK_DIR}"
            return
        fi

        # Option 3: cross-compile from source.
        # Step 1 — build AIDL host tools using the x86 compiler.
        # Temporarily unset ARM cross-compile env and OE detection vars so
        # linux_binder_idl's cmake build produces host (x86) AIDL tools.
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
        # Remove .sdk_ready written by Step 1 so build_arm_binder() does not
        # short-circuit with x86_64 libs still in place.
        rm -f "${LOCAL_BINDER_SDK_DIR}/.sdk_ready"
        echo "ARM build: cross-compiling ARM binder runtime..."
        build_arm_binder
        LOCAL_BINDER_LIB_DIR="${LOCAL_BINDER_SDK_DIR}/lib/binder"
    else
        # Linux native build.
        if [ -d "${binder_toolchain_dir}" ] && [ ! -d "${binder_toolchain_dir}/.git" ]; then
            echo "Removing incomplete toolchain stub before fresh clone..."
            rm -rf "${binder_toolchain_dir}"
        fi
        if [ ! -f "${sdk_ready}" ]; then
            echo -e "${BOLD}${YELLOW}Local Binder SDK not staged. Bootstrapping via rdk-halif-aidl/build_binder.sh${RESET}"
            BINDER_SOURCE_DIR="${binder_toolchain_dir}" "${RDK_HAL_DIR}/build_binder.sh" || {
                echo -e "${BOLD}${RED}ERROR: build_binder.sh failed. Cannot continue.${RESET}"
                exit 1
            }
        fi
        if [ ! -d "${LOCAL_BINDER_INCLUDE_DIR}" ]; then
            echo -e "${BOLD}${RED}ERROR: Local Binder headers not found: ${LOCAL_BINDER_INCLUDE_DIR}${RESET}"
            exit 1
        fi
        if [ ! -d "${LOCAL_BINDER_LIB_DIR}" ]; then
            echo -e "${BOLD}${RED}ERROR: Local Binder libraries not found: ${LOCAL_BINDER_LIB_DIR}${RESET}"
            exit 1
        fi
    fi
}

###############################################################################
# Install Binder service manager, headers and libraries into build/usr/ from
# the rdk-halif-aidl Binder SDK (staged by setup_binder_sdk).
# Must be called after setup_binder_sdk().
###############################################################################
install_linux_binder()
{
    local BINDER_SDK="${LOCAL_BINDER_SDK_DIR}"
    local BINDER_INC="${RDK_HAL_DIR}/out/build"

    echo "Installing binder service manager, headers and libraries..."
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
# Install versioned HALIF headers into build/usr/include/ so the Makefile can
# reference them via the staged build/usr/include path.
# Must be called after resolve_halif_versions().
###############################################################################
install_halif_headers()
{
    echo "Installing HALIF headers..."
    install -d ${BUILD_INSTALL_PREFIX}/include
    if [ -d "${HALIF_AIDL_INC_DIR}" ]; then
        cp -apr "${HALIF_AIDL_INC_DIR}/." "${BUILD_INSTALL_PREFIX}/include/"
    fi
}

###############################################################################
# Function to resolve versioned src/include dirs from rdk-halif-aidl
#
# Version resolution order:
#   1. versions_released.yaml  (rdk-halif-aidl/versions_released.yaml)
#      The cohort-locked build manifest.  Each component listed under the
#      `components:` map is pinned to a specific released snapshot, e.g.:
#
#          default: current
#          components:
#            bootreason: 0.1.0.0
#            firmwareupdate: 0.2.0.0
#            ...
#
#   2. Fallback — if a component is absent from the `components:` map the
#      `default:` value from the same file is used (typically "current").
#
# For each component in HALIF_COMPONENTS the resolved version is used to
# locate pre-generated C++ sources and headers at:
#   <RDK_HAL_DIR>/<component>/<version>/src/
#   <RDK_HAL_DIR>/<component>/<version>/include/
#
# Sets (for each component, last writer wins when HALIF_COMPONENTS has
# multiple entries):
#   HALIF_AIDL_SRC_DIR  – absolute path to the versioned src/ directory
#   HALIF_AIDL_INC_DIR  – absolute path to the versioned include/ directory
###############################################################################
resolve_halif_versions()
{
    # Clone the repository if not already present
    if [ ! -d "$RDK_HAL_DIR" ]; then
        echo "Checking out version: $RDK_HALIF_AIDL_VERSION"
        git clone -b $RDK_HALIF_AIDL_VERSION 'https://github.com/rdkcentral/rdk-halif-aidl.git' $RDK_HAL_DIR
        if [ $? -ne 0 ]; then
            echo -e "${BOLD}╔══════════════════════════════════════════════╗${RESET}"
            echo -e "${BOLD}* ${RED}Failed to clone rdk-halif-aidl. Exiting.${RESET}"
            echo -e "${BOLD}╚══════════════════════════════════════════════╝${RESET}"
            exit 1
        fi
    fi

    local versions_file="${RDK_HAL_DIR}/versions_released.yaml"
    if [ ! -f "$versions_file" ]; then
        echo -e "${BOLD}${RED}ERROR: versions_released.yaml not found: ${versions_file}${RESET}"
        exit 1
    fi

    # Read the fallback version from the `default:` key
    local default_version
    default_version=$(grep -m1 '^default:' "$versions_file" | awk '{print $2}')
    if [ -z "$default_version" ]; then
        default_version="current"
    fi

    for component in "${!HALIF_COMPONENTS[@]}"; do
        # Look up the component in the `components:` map.
        # Entry format (with leading whitespace): "  <component>: <version>"
        local version
        version=$(grep -m1 "^[[:space:]]*${component}:" "$versions_file" | awk '{print $2}')
        if [ -z "$version" ]; then
            echo -e "${BOLD}${YELLOW}WARNING: ${component} not listed in versions_released.yaml, using default: ${default_version}${RESET}"
            version="$default_version"
        fi

        local src_dir="${RDK_HAL_DIR}/${component}/${version}/src"
        local inc_dir="${RDK_HAL_DIR}/${component}/${version}/include"

        if [ ! -d "$src_dir" ]; then
            echo -e "${BOLD}${RED}ERROR: versioned src dir not found: ${src_dir}${RESET}"
            exit 1
        fi
        if [ ! -d "$inc_dir" ]; then
            echo -e "${BOLD}${RED}ERROR: versioned include dir not found: ${inc_dir}${RESET}"
            exit 1
        fi

        echo -e "${BOLD}${GREEN}${component}${RESET}: version=${YELLOW}${version}${RESET} ${CYAN}(versions_released.yaml)${RESET}"
        HALIF_AIDL_SRC_DIR=$(realpath "$src_dir")
        HALIF_AIDL_INC_DIR=$(realpath "$inc_dir")
    done
}

###############################################################################
# Function to build AIDL libraries
###############################################################################
build_aidl_libs()
{
    pushd ${AIDL_LIB_DIR}
    make -j`nproc` \
        HALIF_AIDL_SRC_DIR="${HALIF_AIDL_SRC_DIR}" \
        HALIF_AIDL_INC_DIR="${HALIF_AIDL_INC_DIR}" \
        BINDER_SDK_INCLUDE_DIR="${LOCAL_BINDER_INCLUDE_DIR}" \
        BINDER_SDK_LIB_DIR="${LOCAL_BINDER_LIB_DIR}"
    if [ $? -ne 0 ]; then
        echo -e "${BOLD}╔══════════════════════════════════════════════╗${RESET}"
        echo -e "${BOLD}* ${RED}Failed to build the AIDL Lib. Exiting.${RESET}       ${BOLD}*${RESET}"
        echo -e "${BOLD}╚══════════════════════════════════════════════╝${RESET}"
        exit 1
    fi
    echo "AIDL Build completed successfully."
    popd
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
    # get the latest version of ut-core
    if [ -z "$UT_CORE_VERSION" ]; then
        UT_CORE_VERSION=`git tag | sort -Vr | head -n1`
    fi

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
    # Build the tests — binder and HALIF headers are already staged in build/usr/
    # by install_linux_binder() and install_halif_headers(), so no SDK paths needed.
    make -j`nproc` -C ${TOP} TARGET=$TARGET
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
    make -C ${AIDL_LIB_DIR} clean
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
echo -e "${BOLD}${GREEN}BINDER_SOURCE_DIR      : ${RESET}${YELLOW}$LOCAL_BINDER_TOOLCHAIN_DIR${RESET}"
echo -e "${BOLD}${GREEN}BINDER_SDK_DIR         : ${RESET}${YELLOW}$LOCAL_BINDER_SDK_DIR${RESET}"
echo -e "${BOLD}${GREEN}BINDER_SDK_INCLUDE_DIR : ${RESET}${YELLOW}$LOCAL_BINDER_INCLUDE_DIR${RESET}"
echo -e "${BOLD}${GREEN}BINDER_SDK_LIB_DIR     : ${RESET}${YELLOW}$LOCAL_BINDER_LIB_DIR${RESET}"
echo -e "${BOLD}${GREEN}TARGET                 : ${RESET}${YELLOW}$TARGET${RESET}"

###############################################################################
# Main script execution starts here
###############################################################################
if [ "$1" == "clean" ]; then
    clean_test
elif [ "$1" == "dist_clean" ]; then
    dist_clean_test
elif [ "$1" == "help" ]; then
    help
elif [[ "$1" == [Tt]arget=* ]]; then
    # Validate TARGET
    valid_targets=("linux" "arm")
    if [[ ! " ${valid_targets[*]} " =~ " ${TARGET} " ]]; then
        echo -e "${BOLD}${RED}Invalid Target: ${TARGET}. Valid targets are: ${valid_targets[*]}${RESET}"
        help
        exit 1
    fi
    resolve_halif_versions
    setup_binder_sdk
    install_linux_binder
    install_halif_headers
    build_aidl_libs
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

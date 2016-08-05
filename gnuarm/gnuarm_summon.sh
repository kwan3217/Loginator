#!/bin/bash
# Written by Uwe Hermann <uwe@hermann-uwe.de>, released as public domain.
# Modified by Piotr Esden-Tempski <piotr@esden.net>, released as public domain.

#
# Requirements (example is for Debian, replace package names as needed):
#
# apt-get install flex bison libgmp3-dev libmpfr-dev libncurses5-dev \
# libmpc-dev autoconf texinfo build-essential
#
# Or on Ubuntu Maverick give `apt-get build-dep gcc-4.5` a try.
#

# Stop if any command fails
set -e

##############################################################################
# Default settings section
# You probably want to customize those
# You can also pass them as parameters to the script
##############################################################################
TARGET=arm-none-eabi		# Or: TARGET=arm-elf
PROGRAM_PREFIX=cortex-
WGET="curl -O"              # or wget, if your platform has that instead
#WGET=wget -c --no-passive-ftp
#TARGET=arm-elf		# Or: TARGET=arm-elf
PREFIX=/usr/local/arm	# Install location of your final toolchain
DARWIN_OPT_PATH=/opt/local	# Path in which MacPorts or Fink is installed
# Set to 'sudo' if you need superuser privileges while installing
SUDO=sudo
# Set to 1 to be quieter while running
QUIET=0
# Set to 1 to use linaro gcc instead of the FSF gcc
USE_LINARO=0
# Set to 1 to enable building of OpenOCD
OOCD_EN=0
# Set to 'master' or a git revision number to use instead of stable version
OOCD_GIT=
# Set to 1 to build libstm32 provided by ST
LIBSTM32_EN=0
# Set to 1 to build libopencm3 an open source library for Cortex M3 and simalar
# chips
LIBOPENCM3_EN=1
# Make the gcc default to Cortex-M3
DEFAULT_TO_CORTEX_M3=1
# Override automatic detection of cpus to compile on
#CPUS=1

##############################################################################
# Parsing command line parameters
##############################################################################

while [ $# -gt 0 ]; do
	case $1 in
		TARGET=*)
		TARGET=$(echo $1 | sed 's,^TARGET=,,')
		;;
		PROGRAM_PREFIX=*)
		PROGRAM_PREFIX=$(echo $1 | sed 's,^PROGRAM_PREFIX=,,')
		;;
		PREFIX=*)
		PREFIX=$(echo $1 | sed 's,^PREFIX=,,')
		;;
		DARWIN_OPT_PATH=*)
		DARWIN_OPT_PATH=$(echo $1 | sed 's,^DARWIN_OPT_PATH=,,')
		;;
		SUDO=*)
		SUDO=$(echo $1 | sed 's,^SUDO=,,')
		;;
		QUIET=*)
		QUIET=$(echo $1 | sed 's,^QUIET=,,')
		;;
		USE_LINARO=*)
		USE_LINARO=$(echo $1 | sed 's,^USE_LINARO=,,')
		;;
		OOCD_EN=*)
		OOCD_EN=$(echo $1 | sed 's,^OOCD_EN=,,')
		;;
		OOCD_GIT=*)
		OOCD_GIT=$(echo $1 | sed 's,^OOCD_GIT=,,')
		;;
		LIBSTM32_EN=*)
		LIBSTM32_EN=$(echo $1 | sed 's,^LIBSTM32_EN=,,')
		;;
		LIBOPENCM3_EN=*)
		LIBOPENCM3_EN=$(echo $1 | sed 's,^LIBOPENCM3_EN=,,')
		;;
		DEFAULT_TO_CORTEX_M3=*)
		DEFAULT_TO_CORTEX_M3=$(echo $1 | sed 's,^DEFAULT_TO_CORTEX_M3=,,')
		;;
		CPUS=*)
		CPUS=$(echo $1 | sed 's,^CPUS=,,')
		;;
		*)
		echo "Unknown parameter: $1"
		exit 1
		;;
	esac

	shift # shifting parameter list to access the next one
done

echo "Settings used for this build are:"
echo "TARGET=$TARGET"
echo "PREFIX=$PREFIX"
echo "DARWIN_OPT_PATH=$DARWIN_OPT_PATH"
echo "SUDO=$SUDO"
echo "QUIET=$QUIET"
echo "USE_LINARO=$USE_LINARO"
echo "OOCD_EN=$OOCD_EN"
echo "OOCD_GIT=$OOCD_GIT"
echo "LIBSTM32_EN=$LIBSTM32_EN"
echo "LIBOPENCM3_EN=$LIBOPENCM3_EN"
echo "DEFAULT_TO_CORTEX_M3=$DEFAULT_TO_CORTEX_M3"
echo "CPUS=$CPUS"

##############################################################################
# Version and download url settings section
##############################################################################
if [ ${USE_LINARO} == 0 ] ; then
	# For FSF GCC:
	GCCVERSION=6.1.0
	GCC=gcc-${GCCVERSION}
	GCCURL=http://ftp.gnu.org/gnu/gcc/${GCC}/${GCC}.tar.gz
else
	# For the Linaro GCC:
	GCCRELEASE=4.8-2014.01
	GCCVERSION=${GCCRELEASE}
        LINARO_EXT=xz
	GCC=gcc-linaro-${GCCVERSION}
#              https://launchpad.net/gcc-linaro/4.8/4.8-2014.01/+download/gcc-linaro-4.8-2014.01.tar.xz
	GCCURL=https://launchpad.net/gcc-linaro/4.8/${GCCRELEASE}/+download/${GCC}.tar.${LINARO_EXT}
fi

BINUTILS=binutils-2.26
NEWLIB=newlib-2.4.0.20160527

##############################################################################
# Flags section
##############################################################################

if [ "x${CPUS}" == "x" ]; then
	if which getconf > /dev/null; then
		CPUS=$(getconf _NPROCESSORS_ONLN)
	else
		CPUS=1
	fi

	PARALLEL=-j$((CPUS + 1))
else
	PARALLEL=-j${CPUS}
fi

echo "${CPUS} cpu's detected running make with '${PARALLEL}' flag"

GDBFLAGS=
BINUTILFLAGS=

if [ ${DEFAULT_TO_CORTEX_M3} == 0 ] ; then
	GCCFLAGS=
else
	# To default to the Cortex-M3:
	GCCFLAGS="--with-arch=armv7-m --with-mode=thumb --with-float=hard"
fi

# Pull in the local configuration, if any
if [ -f local.sh ]; then
    . ./local.sh
fi

MAKEFLAGS=${PARALLEL}
TARFLAGS=

if [ ${QUIET} != 0 ]; then
    TARFLAGS=
    MAKEFLAGS="${MAKEFLAGS} -s"
fi

export PATH="${PREFIX}/bin:${PATH}"

SUMMON_DIR=$(pwd)
SOURCES=${SUMMON_DIR}/sources
STAMPS=${SUMMON_DIR}/stamps


##############################################################################
# Tool section
##############################################################################
TAR=tar

##############################################################################
# OS and Tooldetection section
# Detects which tools and flags to use
##############################################################################

case "$(uname)" in
	Linux)
	echo "Found Linux OS."
	;;
	Darwin)
	echo "Found Darwin OS."
	GCCFLAGS="${GCCFLAGS} \
                  --with-gmp=${DARWIN_OPT_PATH} \
	          --with-mpfr=${DARWIN_OPT_PATH} \
	          --with-mpc=${DARWIN_OPT_PATH} \
		  --with-libiconv-prefix=${DARWIN_OPT_PATH}"
	OOCD_CFLAGS="-I/opt/mine/include -I/opt/local/include"
	OOCD_LDFLAGS="-L/opt/mine/lib -L/opt/local/lib"
	if gcc --version | grep llvm-gcc > /dev/null ; then
		echo "Found you are using llvm gcc, switching to clang for gcc compile."
		GCC_CC=clang
	fi
	;;
	MINGW*)
	echo "Found MinGW that means Windows most likely."
	SUDO=""
	;;
	CYGWIN*)
	echo "Found CygWin that means Windows most likely."
	SUDO=""
	;;
	*)
	echo "Found unknown OS. Aborting!"
	exit 1
	;;
esac

##############################################################################
# Building section
# You probably don't have to touch anything after this
##############################################################################

# Fetch a versioned file from a URL
function fetch {
    if [ ! -e ${STAMPS}/$1.fetch ]; then
        log "Downloading $1 sources..."
        ${WGET} $2 && touch ${STAMPS}/$1.fetch
    fi
}

function clone {
    local NAME=$1
    local GIT_REF=$2
    local GIT_URL=$3
    local POST_CLONE=$4
    local GIT_SHA=$(git ls-remote ${GIT_URL} ${GIT_REF} | cut -f 1)

    # It seems that the ref is actually a SHA as it could not be found through ls-remote
    if [ "x${GIT_SHA}" == "x" ]; then
        local GIT_SHA=${GIT_REF}
    fi

    # Setting uppercase NAME variable for future use to the source file name
    eval $(echo ${NAME} | tr "[:lower:]" "[:upper:]")=${NAME}-${GIT_SHA}

    # Clone the repository and do all necessary operations until we get an archive
    if [ ! -e ${STAMPS}/${NAME}-${GIT_SHA}.fetch ]; then
        # Making sure there is nothing in our way
        if [ -e ${NAME}-${GIT_SHA} ]; then
            log "The clone directory ${NAME}-${GIT_SHA} already exists, removing..."
            rm -rf ${NAME}-${GIT_SHA}
        fi
        log "Cloning ${NAME}-${GIT_SHA} ..."
        git clone ${GIT_URL} ${NAME}-${GIT_SHA}
        cd ${NAME}-${GIT_SHA}
        log "Checking out the revision ${GIT_REF} with the SHA ${GIT_SHA} ..."
        git checkout -b sat-branch ${GIT_SHA}
	if [ "x${POST_CLONE}" != "x" ]; then
		log "Running post clone code for ${NAME}-${GIT_SHA} ..."
		${POST_CLONE}
	fi
        log "Removing .git directory from ${NAME}-${GIT_SHA} ..."
        rm -rf .git
        cd ..
        log "Generating source archive for ${NAME}-${GIT_SHA} ..."
        tar cfj ${SOURCES}/${NAME}-${GIT_SHA}.tar.bz2 ${NAME}-${GIT_SHA}
        rm -rf ${NAME}-${GIT_SHA}
        touch ${STAMPS}/${NAME}-${GIT_SHA}.fetch
    fi
}

# Log a message out to the console
function log {
    echo "******************************************************************"
    echo "* $*"
    echo "******************************************************************"
}

# Unpack an archive
function unpack {
    log Unpacking $*
    # Use 'auto' mode decompression.  Replace with a switch if tar doesn't support -a
    ARCHIVE=$(ls ${SOURCES}/$1.tar.*)
    case ${ARCHIVE} in
	*.bz2)
	    echo "archive type bz2"
	    TYPE=j
	    ;;
	*.gz)
	    echo "archive type gz"
	    TYPE=z
	    ;;
	*.xz)
	    echo "archive type xz, hope autodetect works..."
	    TYPE=
	    ;;
	*)
	    echo "Unknown archive type of $1"
	    echo ${ARCHIVE}
	    exit 1
	    ;;
    esac
    ${TAR} xf${TYPE}${TARFLAGS} ${SOURCES}/$1.tar.*
}

# Install a build
function install {
    log $1
    ${SUDO} make ${MAKEFLAGS} $2 $3 $4 $5 $6 $7 $8
}


mkdir -p ${STAMPS} ${SOURCES}

cd ${SOURCES}

fetch ${BINUTILS} http://ftp.gnu.org/gnu/binutils/${BINUTILS}.tar.bz2
fetch ${GCC} ${GCCURL}
fetch ${NEWLIB} ftp://sources.redhat.com/pub/newlib/${NEWLIB}.tar.gz

cd ${SUMMON_DIR}

if [ ! -e build ]; then
    mkdir build
fi

if [ ! -e ${STAMPS}/${BINUTILS}.build ]; then
    unpack ${BINUTILS}
    cd build
    log "Configuring ${BINUTILS}"
    ../${BINUTILS}/configure --target=${TARGET} \
                             --prefix=${PREFIX} \
                             --program-prefix=${TARGET}-${PROGRAM_PREFIX} \
                             --enable-interwork \
                             --enable-multilib \
                             --with-gnu-as \
                             --with-gnu-ld \
                             --disable-nls \
                             --disable-werror \
                             ${BINUTILFLAGS}
    log "Building ${BINUTILS}"
    make ${MAKEFLAGS}
    log "Cleaning old binaries at ${PREFIX}/${TARGET}"
    ${SUDO} rm -rfv ${PREFIX}/${TARGET} ${PREFIX}/bin/${TARGET}-${PROGRAM-PREFIX}* ${PREFIX}/lib/gcc/${TARGET}
    ${SUDO} rm -rfv ${PREFIX}/libexec/gcc/${TARGET} /usr/bin/${TARGET}-${PROGRAM-PREFIX}*
    ${SUDO} rm -rfv ${PREFIX}/share/man/man?/${TARGET}-* /usr/bin/${TARGET}-${PROGRAM-PREFIX}*
    pwd
    install ${BINUTILS} install
    cd ..
    log "Cleaning up ${BINUTILS}"
    touch ${STAMPS}/${BINUTILS}.build
    rm -rf build/* ${BINUTILS}
fi

if [ ! -e ${STAMPS}/${GCC}-${NEWLIB}.build ]; then
    unpack ${GCC}
    unpack ${NEWLIB}

    log "Adding newlib symlink to gcc"
    ln -f -s `pwd`/${NEWLIB}/newlib ${GCC}
    log "Adding libgloss symlink to gcc"
    ln -f -s `pwd`/${NEWLIB}/libgloss ${GCC}

    if [ ${DEFAULT_TO_CORTEX_M3} == 0 ] ; then
	log "Patching gcc to add multilib support"
	cd ${GCC}
#	patch -p0 -i ../patches/patch-gcc-config-arm-t-arm-elf.diff
	cd ..
    fi

    log "Downloading prerequisites"
    cd ${GCC}
    ./contrib/download_prerequisites
    cd ..

    cd build
    if [ "X${GCC_CC}" != "X" ] ; then
	    export GLOBAL_CC=${CC}
	    log "Overriding the default compiler with: \"${GCC_CC}\""
	    export CC=${GCC_CC}
    fi

    log "Configuring ${GCC} and ${NEWLIB}"
    export CFLAGS="${CFLAGS} -D_IEEE_LIBM"
    ../${GCC}/configure --target=${TARGET} \
                      --prefix=${PREFIX} \
                      --program-prefix=${TARGET}-${PROGRAM_PREFIX} \
                      --enable-interwork \
                      --enable-obsolete \
                      --enable-multilib \
                      --enable-languages="c,c++" \
                      --with-newlib \
                      --with-gnu-as \
                      --with-gnu-ld \
                      --disable-nls \
                      --disable-shared \
		      --disable-threads \
		      --disable-newlib-multithread \
		      --disable-newlib-reent-small \
		      --enable-newlib-nano-malloc \
                      --with-headers=newlib/libc/include \
		      --disable-libssp \
		      --disable-libstdcxx-pch \
         	      --disable-libmudflap \
		      --disable-libgomp \
                      --disable-werror \
		      --with-system-zlib \
		      --disable-newlib-supplied-syscalls \
                      --enable-cxx-flags="-fno-exceptions -fno-unwind-tables -ffunction-sections -fdata-sections -fno-use-cxa-atexit -D_IEEE_LIBM" \
		      ${GCCFLAGS}
#	              --disable-hosted-libstdcxx \

    log "Building ${GCC} and ${NEWLIB}"
    make ${MAKEFLAGS}
    #I don't know why, but this install fails the first time and succeeds the second time
    install ${GCC} install || install ${GCC} install
    cd ..
    log "Cleaning up ${GCC} and ${NEWLIB}"

    if [ "X${GCC_CC}" != "X" ] ; then
	    unset CC
	    CC=${GLOBAL_CC}
	    unset GLOBAL_CC
    fi

    touch ${STAMPS}/${GCC}-${NEWLIB}.build
#    rm -rf build/* ${GCC} ${NEWLIB}
fi

cd /usr/local/bin
${SUDO} ln -svf ${PREFIX}/bin/${TARGET}-${PROGRAM_PREFIX}* .


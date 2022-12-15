#!/bin/bash

#
# Copyright SpiRITlab - The Panther Project
# https://github.com/SpiRITlab/Panther
#

# Library versions used
CMAKE_Version=cmake-3.15.0
GMP_Version=gmp-6.2.0
NTL_Version=ntl-11.4.3
HElib_Version=v1.1.0

set_compilers() {
  GCCCompilers=$1
  if [ -z "$GCCCompilers" ] ; then
    echo "Please install GCC-9 or above."
    exit 1
  else
    LatestGCCVersionNum=$(printf "%d\n" "${GCCCompilers[@]}" | sort -rn | head -1)
    # Set compiler version
    CCcompiler=gcc-$LatestGCCVersionNum
    CPPcompiler=g++-$LatestGCCVersionNum
    echo "Set GCC and G++ compiler to $CCcompiler and $CPPcompiler."
  fi
}

OS=$(uname -s)
if [[ "$OS" == "Darwin" ]] ; then
  GCCCompilers=($(ls /usr/local/bin/g++-* | cut -d'-' -f2))
elif [[ "$OS" == "Linux" ]] ; then
  GCCCompilers=($(ls /usr/bin/g++-* | cut -d'-' -f2))
else
  echo 'Unsupported OS'
  exit 1
fi
set_compilers $GCCCompilers



# Directories and files
MARKER=.install_ok
CMAKE_EXE=cmake
ROOT=`pwd`
SOURCE="$ROOT/src"
TARGET="$ROOT/deps"

mkdir -p $TARGET/{include,lib,share,src}

set_trap() {
    set -e
    trap 'last_command=$current_command; current_command=$BASH_COMMAND' DEBUG
    trap 'echo "\"${last_command}\" command failed with exit code $?."' EXIT
}

set_trap

# =============================================================================
# Functions to minimize code redundancy
# =============================================================================
install_cmake() {
    echo "Installing $CMAKE..."
    # wget https://github.com/Kitware/CMake/releases/download/v$(echo $CMAKE_Version | cut -d'-' -f2)/$CMAKE_Version.tar.gz
    # tar -zxvf $CMAKE_Version.tar.gz
    # rm $CMAKE_Version.tar.gz
    # mv $CMAKE_Version $CMAKE
    # cd $CMAKE
    # ./bootstrap --prefix=$TARGET
    # make; make install
    # echo "Installing $CMAKE... (DONE)"
    # touch $Marker # add the marker
    # cd ..
}

install_googletest() {
    echo "Installing $GoogleTEST..."
    # git clone https://github.com/google/googletest.git $GoogleTEST
    # cd $GoogleTEST
    # $CMAKE_EXE -DCMAKE_CXX_COMPILER=$CPPcompiler -DCMAKE_INSTALL_PREFIX=$TARGET .
    # make CXX=$CPPcompiler LD=$CPPcompiler; make install
    # echo "Installing $GoogleTEST... (DONE)"
    # touch $MARKER # add the marker
    # cd ..
}

install_gmp() {
    echo "Installing $GMP..."
    wget https://ftp.gnu.org/gnu/gmp/$GMP_Version.tar.bz2
    tar jxf $GMP_Version.tar.bz2
    rm $GMP_Version.tar.bz2
    mv $GMP_Version $GMP
    cd $GMP
    ./configure --prefix=$TARGET --exec-prefix=$TARGET
    make; make install
    echo "Installing $GMP... (DONE)"
    touch $MARKER # add the marker
    cd ..
}

install_ntl() {
    echo "Installing $NTL..."
    wget http://www.shoup.net/ntl/$NTL_Version.tar.gz
    tar xzf $NTL_Version.tar.gz
    rm $NTL_Version.tar.gz
    mv $NTL_Version $NTL
    cd $NTL/src
    ./configure TUNE=x86 DEF_PREFIX=$TARGET NTL_THREADS=on NTL_THREAD_BOOST=on NTL_GMP_LIP=on NATIVE=off CXX=$CPPcompiler
    make CXX=$CPPcompiler CXXFLAGS="-fPIC -O3"
    make install
    echo "Installing $NTL... (DONE)"
    cd ..
    touch $MARKER # add the marker
    cd ..
}

install_helib() {
    echo "Installing $HElib..."
    git clone https://github.com/homenc/HElib.git $HElib
    cd $HElib
    git checkout $HElib_Version
    $CMAKE_EXE -DCMAKE_CXX_COMPILER=$CPPcompiler -DCMAKE_INSTALL_PREFIX=$TARGET .
    make CC=$GCCcompiler LD=$CPPcompiler LDLIBS+=-L$TARGET/lib CFLAGS+=-I$TARGET/include CFLAGS+=-fPIC
    mkdir -p $TARGET/include/HElib/
    cp -R include/helib $TARGET/include/
    cp lib/*.a $TARGET/lib/
    echo "Installing $HElib... (DONE)"
    touch $MARKER # add the marker
    cd ..
}



# =============================================================================
# Install dependencies
# =============================================================================

cd "$TARGET/src"

export LD_LIBRARY_PATH=$TARGET/lib

# determine whether this script is ran inside a docker container
if [ ! -f "/proc/1/cgroup" ] || [ "$(grep 'docker\|lxc' /proc/1/cgroup)" == "" ] ; then
  # if not, install dependencies into a self-contained folder
  DEPS_ROOT=$TARGET
else
  # if docker container, install dependencies into /usr/local
  DEPS_ROOT=/usr/local
fi


installMinRequiredCmake() {
  CMAKE="CMAKE"
  CMAKE_EXE=$TARGET/bin/cmake
  if [ -d $CMAKE ]; then
    if [ ! -f $CMAKE/$Marker ]; then
      rm -rf $CMAKE # remove the folder
      install_cmake
    else
      echo "$CMAKE library already installed"
    fi
  else
    install_cmake
  fi
}


echo "Checking CMake version...(Minimum required version: $(echo $CMAKE_Version | cut -d'-' -f2))"
if [ ! -z "$(cmake --version | grep 'version')" ]; then
  installedCmakeVersion=$(cmake --version | grep 'version' | cut -d' ' -f3)
  installedCmakeMajorVersion=$(echo $installedCmakeVersion | cut -d'.' -f1)
  installedCmakeMinorVersion=$(echo $installedCmakeVersion | cut -d'.' -f2)
  requiredMinCmakeMajorVersion=$(echo $CMAKE_Version | cut -d'-' -f2 | cut -d'.' -f1)
  requiredMinCmakeMinorVersion=$(echo $CMAKE_Version | cut -d'-' -f2 | cut -d'.' -f2)
  if [ "$installedCmakeMajorVersion" -lt "$requiredMinCmakeMajorVersion" ] \
    || [ "$installedCmakeMinorVersion" -lt "$requiredMinCmakeMinorVersion" ]; then
        echo "The installed version of CMake (current version: $installedCmakeVersion) is older than required. Perform install newer version."
        installMinRequiredCmake
  else
        echo "Minimum requirement for CMake is satisfied! (Current Version: $installedCmakeVersion)"
  fi
else
  installMinRequiredCmake
fi

# Google Test is a unit testing library for the C++ programming language,
# based on the xUnit architecture.
GoogleTEST="GoogleTEST"
if [ -d $GoogleTEST ]; then
    if [ ! -f $GoogleTEST/$MARKER ]; then
        rm -rf $GoogleTEST # remove the folder
        install_googletest
    else
        echo "$GoogleTEST already installed"
    fi
else
    install_googletest
fi

# The GMP package contains math libraries. These have useful functions
# for arbitrary precision arithmetic.
GMP="GMP"
if [ -d $GMP ]; then
    if [ ! -f $GMP/$MARKER ]; then
        rm -rf $GMP # remove the folder
        install_gmp
    else
        echo "$GMP already installed"
    fi
else
    install_gmp
fi


# NTL is a C++ library for doing number theory. NTL supports arbitrary
# length integer and arbitrary precision floating point arithmetic, finite
# fields, vectors, matrices, polynomials, lattice basis reduction and basic
# linear algebra.
NTL="NTL"
if [ -d $NTL ]; then
    if [ ! -f $NTL/$MARKER ]; then
        rm -rf $NTL # remove the folder
        install_ntl
    else
        echo "$NTL already installed"
    fi
else
    install_ntl
fi


# HElib is a software library that implements homomorphic encryption (HE).
HElib="HElib"
if [ -d $HElib ]; then
     if [ ! -f $HElib/$MARKER ]; then
        rm -rf $HElib # remove the folder
        install_helib
    else
        echo "$HElib already installed"
    fi
else
    install_helib
fi



# =============================================================================
# Generate JNI from native code
# =============================================================================

cd $ROOT

$CMAKE_EXE -DCMAKE_PREFIX_PATH=$TARGET -DCMAKE_CXX_COMPILER=$CPPcompiler .

trap EXIT

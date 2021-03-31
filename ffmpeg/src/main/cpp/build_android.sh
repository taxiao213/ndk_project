#!/bin/bash
export NDK_HOME=/home/build/workspace/hqq/vision/vision-app/ndk/android-ndk-r21b
export PLATFORM_VERSION=android-21

# 编译工具链目录，ndk17版本以上用的是clang，以下是gcc
TOOLCHAIN=$NDK_HOME/toolchains/llvm/prebuilt/darwin-x86_64
# 版本号
API=21
# 交叉编译树的根目录(查找相应头文件和库用)
SYSROOT="${TOOLCHAIN}/sysroot"

CC="${TOOL_PREFIX}${API}-clang"
CXX="${TOOL_PREFIX}${API}-clang++"

function build
{
	echo "start build ffmpeg for $ARCH"
	./configure  \
	--prefix=$PREFIX \
	--target-os=android \
	--arch=$ARCH \
	--disable-doc \
	--enable-shared \
	--disable-static \
	--disable-yasm \
	--disable-asm \
	--disable-symver \
	--enable-gpl \
	--disable-ffmpeg \
	--disable-ffplay \
	--disable-ffprobe \
	--cpu=$CPU \
	--cc=$CC \
	--cxx=$CXX \
	--enable-cross-compile \
	--sysroot=$SYSROOT \
	--enable-small \
	--extra-cflags="-Os -fpic $ADDI_CFLAGS" \
	--extra-ldflags="$ADDI_LDFLAGS" \
	$ADDITIONAL_CONFIGURE_FLAG
	make clean
	make -j8
	make install
	echo "build ffmpeg for $ARCH finished"
}

#arm
ARCH="arm"
CPU="arm"
PREFIX=$(pwd)/android/$ARCH
TOOL_CPU_NAME="arm"
ADDI_CFLAGS="-marm"
TOOL_PREFIX="$TOOLCHAIN/bin/${TOOL_CPU_NAME}-linux-androideabi"
build

# x86
#ARCH="x86"
#CPU="x86"
#PREFIX=$(pwd)/android/$ARCH
#TOOL_CPU_NAME="i686"
#TOOL_PREFIX="$TOOLCHAIN/bin/${TOOL_CPU_NAME}-linux-android"
#ADDI_CFLAGS="-march=i686 -mtune=intel -mssse3 -mfpmath=sse -m32"
#build

# x86_64，这个指令集最低支持api21
#ARCH="x86_64"
#CPU="x86_64"
#PREFIX=$(pwd)/android/$ARCH
#TOOL_CPU_NAME="x86_64"
#TOOL_PREFIX="$TOOLCHAIN/bin/${TOOL_CPU_NAME}-linux-android"
#ADDI_CFLAGS="-march=$CPU"
#build

# armv7-a
#ARCH="arm"
#CPU="armv7-a"
#PREFIX=$(pwd)/android/$ARCH
#TOOL_CPU_NAME="armv7a"
#TOOL_PREFIX="$TOOLCHAIN/bin/${TOOL_CPU_NAME}-linux-androideabi"
#ADDI_CFLAGS="-march=$CPU"
#build

#arm64-v8a，这个指令集最低支持api21
#ARCH="aarch64"
#CPU="armv8-a"
#PREFIX=$(pwd)/android/$ARCH
#TOOL_CPU_NAME="aarch64"
#TOOL_PREFIX="$TOOLCHAIN/bin/${TOOL_CPU_NAME}-linux-android"
#ADDI_CFLAGS="-march=$CPU"
#build


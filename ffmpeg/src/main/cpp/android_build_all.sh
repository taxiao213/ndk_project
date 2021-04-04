#!/bin/bash
export NDK_HOME=/home/build/workspace/hqq/vision/vision-app/ndk/android-ndk-r14b
export PLATFORM_VERSION=android-21
function build
{
	echo "start build ffmpeg for $ARCH"
	./configure --target-os=linux \
	--prefix=$PREFIX --arch=$ARCH \
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
	--disable-ffserver \
	--cross-prefix=$CROSS_COMPILE \
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
ARCH=arm
ARCH_PREFIX=arm
CPU=armv7-a
PREFIX=$(pwd)/android_all/$ARCH
TOOLCHAIN=$NDK_HOME/toolchains/$ARCH_PREFIX-linux-androideabi-4.9/prebuilt/linux-x86_64
CROSS_COMPILE=$TOOLCHAIN/bin/$ARCH_PREFIX-linux-androideabi-
ADDI_CFLAGS="-march=$CPU"
SYSROOT=$NDK_HOME/platforms/$PLATFORM_VERSION/arch-$ARCH/
#build

#arm64
ARCH=arm64
ARCH_PREFIX=aarch64
CPU=armv8-a
PREFIX=$(pwd)/android_all/$ARCH
TOOLCHAIN=$NDK_HOME/toolchains/$ARCH_PREFIX-linux-android-4.9/prebuilt/linux-x86_64
CROSS_COMPILE=$TOOLCHAIN/bin/$ARCH_PREFIX-linux-android-
ADDI_CFLAGS="-march=$CPU"
SYSROOT=$NDK_HOME/platforms/$PLATFORM_VERSION/arch-$ARCH/
build

#mips
ARCH=mips
ARCH_PREFIX=mipsel
#CPU=
PREFIX=$(pwd)/android_all/$ARCH
TOOLCHAIN=$NDK_HOME/toolchains/$ARCH_PREFIX-linux-android-4.9/prebuilt/linux-x86_64
CROSS_COMPILE=$TOOLCHAIN/bin/$ARCH_PREFIX-linux-android-
ADDI_CFLAGS=""
SYSROOT=$NDK_HOME/platforms/$PLATFORM_VERSION/arch-$ARCH/
#build

#mips64
ARCH=mips64
ARCH_PREFIX=mips64el
#CPU=
PREFIX=$(pwd)/android_all/$ARCH
TOOLCHAIN=$NDK_HOME/toolchains/$ARCH_PREFIX-linux-android-4.9/prebuilt/linux-x86_64
CROSS_COMPILE=$TOOLCHAIN/bin/$ARCH_PREFIX-linux-android-
ADDI_CFLAGS=""
SYSROOT=$NDK_HOME/platforms/$PLATFORM_VERSION/arch-$ARCH/
#build

#x86
ARCH=x86
ARCH_PREFIX=x86-4.9
CPU=x86
PREFIX=$(pwd)/android_all/$ARCH
TOOLCHAIN=$NDK_HOME/toolchains/$ARCH_PREFIX/prebuilt/linux-x86_64
CROSS_COMPILE=$TOOLCHAIN/bin/i686-linux-android-
ADDI_CFLAGS="-march=i686 -mtune=intel -mssse3 -mfpmath=sse -m32"
SYSROOT=$NDK_HOME/platforms/$PLATFORM_VERSION/arch-$ARCH/
#build

#x86_64
ARCH=x86_64
ARCH_PREFIX=x86_64-4.9
CPU=x86_64
PREFIX=$(pwd)/android_all/$ARCH
TOOLCHAIN=$NDK_HOME/toolchains/$ARCH_PREFIX/prebuilt/linux-x86_64
CROSS_COMPILE=$TOOLCHAIN/bin/x86_64-linux-android-
ADDI_CFLAGS="-march=$CPU"
SYSROOT=$NDK_HOME/platforms/$PLATFORM_VERSION/arch-$ARCH/
#build

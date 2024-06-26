SDL2_CFG+= --enable-loadso
SDL2_CFG+= --enable-joystick
SDL2_CFG+= --enable-video-mali
SDL2_CFG+= --enable-video-opengl
SDL2_CFG+= --enable-video-opengles
SDL2_CFG+= --enable-video-opengles2

MOD      = a30
SDL2_INC = -I/opt/miyoo_a30/arm-linux-gnueabihf/sysroot/usr/include/SDL2

export CROSS=/opt/miyoo_a30/bin/arm-linux-
export CC=${CROSS}gcc
export AR=${CROSS}ar
export AS=${CROSS}as
export LD=${CROSS}ld
export CXX=${CROSS}g++
export HOST=arm-linux

include Makefile.mk

.PHONY: all
all: sdl2 example

.PHONY: cfg
cfg:
	cd sdl2 && ./autogen.sh && MOD=$(MOD) ./configure ${SDL2_CFG} --host=${HOST}

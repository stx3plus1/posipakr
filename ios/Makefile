ARCHS=armv6 armv7 arm64 arm64e
PACKAGE_VERSION=1.0

TARGET := iphone:clang:latest:5.1
TARGET_ARCHS = armv6 armv7

TARGET_64 := iphone:clang:latest:7.0
TARGET_ARCHS_64 = arm64

TARGET_64E := iphone:clang:latest:12.0
TARGET_ARCHS_64E = arm64e

include $(THEOS)/makefiles/common.mk

TOOL_NAME = posipakr

posipakr_FILES = $(wildcard ../src/*.c)
posipakr_CFLAGS = -fobjc-arc
posipakr_CODESIGN_FLAGS = -Sentitlements.plist
posipakr_INSTALL_PATH = /usr/local/bin

include $(THEOS_MAKE_PATH)/tool.mk

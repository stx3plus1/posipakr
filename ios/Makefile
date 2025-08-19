PACKAGE_VERSION=1.0
TARGET := iphone:clang:5.1:5.0

include $(THEOS)/makefiles/common.mk

TOOL_NAME = posipakr

posipakr_FILES = $(wildcard ../src/*.c)
posipakr_CFLAGS = -fobjc-arc
posipakr_CODESIGN_FLAGS = -Sentitlements.plist
posipakr_INSTALL_PATH = /usr/local/bin

include $(THEOS_MAKE_PATH)/tool.mk

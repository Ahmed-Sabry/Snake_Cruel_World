# Auto-detect Homebrew prefix (Apple Silicon: /opt/homebrew, Intel: /usr/local)
HOMEBREW_PREFIX := $(shell brew --prefix)

CC := clang++
CFLAGS := $(CFLAGS:-s=) -isystem $(HOMEBREW_PREFIX)/include

LIB_DIRS := \
	$(HOMEBREW_PREFIX)/lib

INCLUDE_DIRS :=

BUILD_FLAGS :=

MACOS_FRAMEWORK_PATHS := \
	/Library/Frameworks \
	$(HOMEBREW_PREFIX)/Frameworks

# Name, no extension (eg. CoreFoundation, ogg)
MACOS_FRAMEWORKS := \
	CoreFoundation

# Icon .png
PRODUCTION_MACOS_ICON := sfml

PRODUCTION_DEPENDENCIES := \
	$(PRODUCTION_DEPENDENCIES)

PRODUCTION_MACOS_BUNDLE_DEVELOPER := developer
PRODUCTION_MACOS_BUNDLE_DISPLAY_NAME := Snake Cruel World
PRODUCTION_MACOS_BUNDLE_NAME := Snake Cruel World
PRODUCTION_MACOS_MAKE_DMG := true
PRODUCTION_MACOS_BACKGROUND := dmg-background

PRODUCTION_MACOS_DYLIBS := \
	$(HOMEBREW_PREFIX)/lib/libsfml-graphics.2.6 \
	$(HOMEBREW_PREFIX)/lib/libsfml-audio.2.6 \
	$(HOMEBREW_PREFIX)/lib/libsfml-network.2.6 \
	$(HOMEBREW_PREFIX)/lib/libsfml-window.2.6 \
	$(HOMEBREW_PREFIX)/lib/libsfml-system.2.6

# Path, no extension (eg. /Library/Frameworks/ogg)
PRODUCTION_MACOS_FRAMEWORKS :=

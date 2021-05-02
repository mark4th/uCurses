# We are not using any builtins
MAKEFLAGS:= --warn-undefined-variables \
			--no-builtin-rules \
			--no-builtin-variables \
			--output-sync=target \
			--silent

.ONESHELL:


CC := $(shell which clang >/dev/null && CC=$${CC:-clang}; echo $$CC)
CC_LD := $(shell which lld >/dev/null && CC_LD=$${CC_LLD:-lld}; echo $$CC_LD)

PROJECT_ROOT := $(PWD)

ARCH_MESON := $(PROJECT_ROOT)/arch.meson
ARCH_BUILD := $(ARCH_MESON)/arch.build
BUILD_CONFIG := packaging/linux/distro/arch
MESON_OPT :=
NINJA_OPT := --verbose
MESON_BUILD_IN := $(BUILD_CONFIG)/meson.build
MESON_BUILD := $(ARCH_MESON)/meson.build

export CC
export CC_LD
export PROJECT_ROOT
export ARCH_MESON

_ = echo '[$@]'; set -euo pipefail;

default: build
	$_

.vars:
	$_
	echo CC=$(CC)
	echo CC_LD=$(CC_LD)
	echo ARCH_MESON=$(ARCH_MESON)
	echo ARCH_BUILD=$(ARCH_BUILD)
	echo BUILD_CONFIG=$(BUILD_CONFIG)
	echo MESON_OPT=$(MESON_OPT)
	echo NINJA_OPT=$(NINJA_OPT)
	echo MESON_BUILD_IN=$(MESON_BUILD_IN)
	echo MESON_BUILD=$(MESON_BUILD)

format:
	$_
	./clang-tidy.sh


clean:
	$_
	rm -rf $(ARCH_MESON)


$(MESON_BUILD): $(MESON_BUILD_IN)
	$_
	make --silent clean
	cp -av $(BUILD_CONFIG)/ $(ARCH_MESON)/
	cd $(ARCH_MESON) && meson setup $(ARCH_BUILD) $(MESON_OPT)


build: $(MESON_BUILD)
	$_
	cd $(ARCH_BUILD) && ninja $(NINJA_OPT)


rebuild: clean build
	$_

install: $(MESON_BUILD)
	$_
	cd $(ARCH_BUILD) && sudo ninja $(NINJA_OPT) install


clean-install: clean install
	$_

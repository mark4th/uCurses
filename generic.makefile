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

MESON := $(PROJECT_ROOT)/meson
BUILD := $(MESON)/build
BUILD_CONFIG := .
MESON_OPT :=
NINJA_OPT := --verbose
MESON_BUILD_IN := $(BUILD_CONFIG)/meson.build
MESON_BUILD := $(MESON)/meson.build

export CC
export CC_LD
export PROJECT_ROOT
export MESON

_ = echo '[$@]'; set -euo pipefail;

default: build
	$_

.vars:
	$_
	echo CC=$(CC)
	echo CC_LD=$(CC_LD)
	echo MESON=$(MESON)
	echo BUILD=$(BUILD)
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
	rm -rf $(MESON)


$(MESON_BUILD): $(MESON_BUILD_IN)
	make --silent clean
	$_
	mkdir -p  $(MESON)/
	cp -av $(MESON_BUILD_IN) $(MESON)/
	cd $(MESON) && meson setup $(BUILD) $(MESON_OPT)


build: $(MESON_BUILD)
	$_
	cd $(BUILD) && ninja $(NINJA_OPT)


rebuild: clean build
	$_

install: $(MESON_BUILD)
	$_
	cd $(BUILD) && sudo ninja $(NINJA_OPT) install


clean-install: clean install
	$_

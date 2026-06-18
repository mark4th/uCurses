# We are not using any builtins
SHELL := /bin/bash

MAKEFLAGS:= --warn-undefined-variables \
	--no-builtin-rules \
	--no-builtin-variables \
	--output-sync=target \
	--silent 


.ONESHELL:


CC := $(shell which clang >/dev/null && CC=$${CC:-clang}; echo $$CC)
CC_LD := $(shell which lld >/dev/null && CC_LD=$${CC_LLD:-lld}; echo $$CC_LD)

PROJECT_ROOT := $(CURDIR)

BUILD := $(PROJECT_ROOT)/build
MESON_OPT :=
NINJA_OPT := --verbose
BUILD_TOUCHED := $(BUILD)/.touched
MESON_BUILD := meson.build
MESON_OPTIONS := meson_options.txt

export CC
export CC_LD

_ = echo '[$@]'; set -euo pipefail;

default: .build
	$_

.vars:
	$_
	echo CC=$(CC)
	echo CC_LD=$(CC_LD)
	echo BUILD=$(BUILD)
	echo MESON_OPT=$(MESON_OPT)
	echo NINJA_OPT=$(NINJA_OPT)
	echo MESON_BUILD=$(MESON_BUILD)
	echo MESON_OPTIONS=$(MESON_OPTIONS)
	echo BUILD_TOUCHED=$(BUILD_TOUCHED)

format:
	$_
	./clang-tidy.sh


clean:
	$_
	rm -rf $(BUILD)


$(BUILD_TOUCHED): $(MESON_BUILD) $(MESON_OPTIONS) generic.makefile Makefile
	make clean
	$_
	meson setup $(BUILD) $(MESON_OPT)
	touch $(BUILD_TOUCHED)


.build: $(BUILD_TOUCHED)
	$_
	cd $(BUILD) && ninja $(NINJA_OPT)


rebuild: clean .build
	$_

install: .build
	$_
	cd $(BUILD) && sudo ninja $(NINJA_OPT) install
	sudo cp ../h/uC*.h /usr/local/include

clean-install: clean install
	$_

scan:
	rm -f $(BUILD_TOUCHED)
	scan-build --use-cc=$(CC) make .build

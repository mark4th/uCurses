include generic.makefile

project = uCurses
shared_object=lib$(project).so
library = $(ARCH_BUILD)/$(shared_object)

help:
	$_
	set -x
	cd $(ARCH_BUILD)
	./$(shared_object) --help

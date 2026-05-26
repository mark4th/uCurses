include generic.makefile

project = uCurses
shared_object=lib$(project).so

examples: .build
	$_
	cd $(BUILD) && ninja examples

help:
	$_
	set -x
	cd $(BUILD)
	./$(shared_object) --help

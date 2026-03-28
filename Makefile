include generic.makefile

project = uCurses
shared_object=lib$(project).so

help:
	$_
	set -x
	cd $(BUILD)
	./$(shared_object) --help

include generic.makefile

project = uCurses
shared_object=lib$(project).so

examples: .build
	$_
	cd $(BUILD) && ninja examples

tests: .build
	$_
	cd $(BUILD) && ninja meson-test-prereq && meson test --no-rebuild

help:
	$_
	set -x
	cd $(BUILD)
	./$(shared_object) --help

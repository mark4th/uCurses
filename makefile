include generic.makefile

project = uCurses
shared_object=lib$(project).so

help:
	$_
	set -x
	cd $(BUILD)
	./$(shared_object) --help

test:
	$_
	CHORDTEST_TEST_TAG=test-uCurses ./build/test-uCurses.src.app

include generic.makefile

project = uCurses
shared_object=lib$(project).so
example_bins = demo scroll console widgets window raycast dots mandel borders lion list_demo

.PHONY: clean-examples examples tests help

clean: clean-examples

clean-examples:
	$_
	for exe in $(example_bins); do
		rm -f "$(PROJECT_ROOT)/example/$$exe"
	done

examples: .build
	$_
	$(MAKE) clean-examples
	cd $(BUILD)
	ninja examples
	for exe in example/*; do
		if [[ -f "$$exe" && -x "$$exe" ]]; then
			cp -f "$$exe" "$(PROJECT_ROOT)/example/"
		fi
	done

tests: .build
	$_
	cd $(BUILD) && ninja meson-test-prereq && meson test --no-rebuild

help:
	$_
	set -x
	cd $(BUILD)
	./$(shared_object) --help

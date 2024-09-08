SUBDIRS = bin lib/nolifebar

.PHONY: test

test:
	@echo "---> Running tests for lib/functions"
	./test/01-functions.spec.bash

test-nolifebar-fn-xft:
	$(MAKE) -C bin test-nolifebar-fn-xft

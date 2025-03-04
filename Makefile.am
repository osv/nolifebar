SUBDIRS = bin lib/nolifebar

.PHONY: test

test:
	@echo "---> Running tests for lib/functions"
	-./test/01-functions.spec.bash
	@echo "---> Running tests for ./bin/nolifebar-replacer"
	-./test/02-replacer.spec.bash

test-nolifebar-fn-xft:
	$(MAKE) -C bin test-nolifebar-fn-xft

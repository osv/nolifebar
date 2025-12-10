SUBDIRS = bin lib/nolifebar share/nolifebar

.PHONY: test test-01-functions test-02-replacer

test: test-01-functions test-02-replacer

test-01-functions:
	@echo "---> Running tests for lib/functions"
	-./test/01-functions.spec.bash

test-02-replacer:
	@echo "---> Running tests for ./bin/nolifebar-replacer"
	-./test/02-replacer.spec.bash

test-nolifebar-fn-xft:
	$(MAKE) -C bin test-nolifebar-fn-xft


all:	prerequisites generator examples

.PHONY:	prerequisites
prerequisites:
	./bin/check-prerequisites.sh

.PHONY:	generator
generator:
	$(MAKE) -C generator

.PHONY: examples
examples:
	$(MAKE) -C examples/vamp-example-plugins clean em
	$(MAKE) -C examples/vamp-test-plugin clean em
	$(MAKE) -C examples/vamp-example-plugins test
	$(MAKE) -C examples/vamp-test-plugin test

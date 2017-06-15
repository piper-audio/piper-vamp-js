
all:	prerequisites generator examples

prerequisites:
	./bin/check-prerequisites.sh

clean:
	$(MAKE) -C generator clean
	$(MAKE) -C examples/vamp-example-plugins clean
	$(MAKE) -C examples/vamp-test-plugin clean

.PHONY:	generator
generator:
	$(MAKE) -C generator

.PHONY: examples
examples:
	$(MAKE) -C examples/vamp-example-plugins em
	$(MAKE) -C examples/vamp-test-plugin em
	$(MAKE) -C examples/vamp-example-plugins test
	$(MAKE) -C examples/vamp-test-plugin test

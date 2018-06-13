
all:	prerequisites generator examples-wasm

prerequisites:
	./bin/check-prerequisites.sh

clean:
	$(MAKE) -C generator clean
	$(MAKE) -C examples/vamp-example-plugins clean
	$(MAKE) -C examples/vamp-test-plugin clean

.PHONY:	generator
generator:
	$(MAKE) -C generator

.PHONY: examples-em
examples-em:
	$(MAKE) -C examples/vamp-example-plugins em
	$(MAKE) -C examples/vamp-test-plugin em
	$(MAKE) -C examples/vamp-example-plugins test-em
	$(MAKE) -C examples/vamp-test-plugin test-em

.PHONY: examples-wasm
examples-wasm:
	$(MAKE) -C examples/vamp-example-plugins wasm
	$(MAKE) -C examples/vamp-test-plugin wasm
	$(MAKE) -C examples/vamp-example-plugins test-wasm
	$(MAKE) -C examples/vamp-test-plugin test-wasm

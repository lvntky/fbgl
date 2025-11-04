CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lSDL2 -ldl -lm

.PHONY: all clean test

all: libfbgl_preload.so fbgl_viewer fbgl_run.sh

libfbgl_preload.so: fbgl_preload.c
	@echo "Building preload library..."
	$(CC) -shared -fPIC $(CFLAGS) $< -ldl -o $@

fbgl_viewer: fbgl_viewer.c
	@echo "Building SDL viewer..."
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

fbgl_run.sh:
	@echo "Making runner script executable..."
	@chmod +x fbgl_run.sh

test: all test_fbgl
	@echo "Running test program..."
	./fbgl_run.sh ./test_fbgl

test_fbgl: test_fbgl.c fbgl.h
	@echo "Compiling test program..."
	$(CC) $(CFLAGS) test_fbgl.c -lm -o test_fbgl

clean:
	@echo "Cleaning up..."
	rm -f libfbgl_preload.so fbgl_viewer test_fbgl
	@# Clean up shared memory
	@ipcrm -M $$(ftok /tmp F 2>/dev/null | awk '{print $$NF}') 2>/dev/null || true

install:
	@echo "Installing to /usr/local/bin..."
	sudo cp libfbgl_preload.so /usr/local/lib/
	sudo cp fbgl_viewer /usr/local/bin/
	sudo cp fbgl_run.sh /usr/local/bin/fbgl-run
	@echo "Done! You can now run: fbgl-run ./your_program"

uninstall:
	@echo "Uninstalling..."
	sudo rm -f /usr/local/lib/libfbgl_preload.so
	sudo rm -f /usr/local/bin/fbgl_viewer
	sudo rm -f /usr/local/bin/fbgl-run
	@echo "Done!"

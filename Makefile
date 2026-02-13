BUILD_DIR ?= build
PREFIX ?= /usr/local

$(BUILD_DIR)/idleclicker: $(BUILD_DIR) main.c platform_linux.c $(BUILD_DIR)/libraylib.a icon_data.h
	gcc -Os -c platform_linux.c -o $(BUILD_DIR)/platform_linux.o
	gcc -Os -o $(BUILD_DIR)/idleclicker main.c $(BUILD_DIR)/platform_linux.o \
		-Iraylib/src -L$(BUILD_DIR) \
		-Wl,-Bstatic -lraylib \
		-Wl,--start-group -lX11 -lXi -lXtst -lXext -lxcb -lXau -lXdmcp -Wl,--end-group \
		-lpthread -Wl,-Bdynamic -lm -ldl -lc $(LDFLAGS)
	strip $(BUILD_DIR)/idleclicker

windows: clean
	docker build -t idleclicker-mingw -f Dockerfile.mingw .
	docker run --rm -v $(PWD):/work --user $(shell id -u):$(shell id -g) idleclicker-mingw /work/build-windows.sh

linux: clean
	docker build -t idleclicker-linux -f Dockerfile.linux .
	docker run --rm -v $(PWD):/work --user $(shell id -u):$(shell id -g) idleclicker-linux make
	cp $(BUILD_DIR)/idleclicker $(BUILD_DIR)/idleclicker.glibc.x11.linux.x86_64

clean:
	make -C raylib/src clean
	rm -rf $(BUILD_DIR)

install:
	install -d $(PREFIX)/bin $(PREFIX)/share/applications $(PREFIX)/share/icons
	install -m 755 $(BUILD_DIR)/idleclicker $(PREFIX)/bin/idleclicker
	install -m 755 idleclicker.desktop $(PREFIX)/share/applications/idleclicker.desktop
	install -m 644 idleclicker.png $(PREFIX)/share/icons/idleclicker.png

.PHONY: clean windows install

$(BUILD_DIR):
	$(MAKE) -C raylib/src clean
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/libraylib.a:
	PLATFORM=PLATFORM_DESKTOP_GLFW $(MAKE) -C raylib/src
	cp raylib/src/libraylib.a $(BUILD_DIR)/libraylib.a

icon_data.h: idleclicker.png
	xxd -i idleclicker.png > icon_data.h


BUILD_DIR ?= build
PREFIX ?= /usr/local

$(BUILD_DIR)/idleclicker: $(BUILD_DIR) main.c platform_linux.c $(BUILD_DIR)/libraylib.a icon_data.h
	gcc -c platform_linux.c -o $(BUILD_DIR)/platform_linux.o
	gcc -Os -o $(BUILD_DIR)/idleclicker main.c $(BUILD_DIR)/platform_linux.o -Iraylib/src -L$(BUILD_DIR) -lraylib -lm -lX11 -lXi -lXtst -lpthread

windows:
	docker build -t idleclicker-mingw .
	docker run --rm -v $(PWD):/work --user $(id -u):$(id -g) idleclicker-mingw /work/build-windows.sh

clean:
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
	$(MAKE) -C raylib/src
	cp raylib/src/libraylib.a $(BUILD_DIR)/libraylib.a

icon_data.h: idleclicker.png
	xxd -i idleclicker.png > icon_data.h


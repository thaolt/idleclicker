BUILD_DIR ?= build

$(BUILD_DIR)/idleclicker: $(BUILD_DIR) main.c platform_linux.c $(BUILD_DIR)/libraylib.a
	gcc -c platform_linux.c -o $(BUILD_DIR)/platform_linux.o
	gcc -Os -o $(BUILD_DIR)/idleclicker main.c $(BUILD_DIR)/platform_linux.o -Iraylib/src -L$(BUILD_DIR) -lraylib -lm -lX11 -lXi -lXtst -lpthread

clean:
	rm -rf $(BUILD_DIR)

.PHONY: clean

$(BUILD_DIR):
	$(MAKE) -C raylib/src clean
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/libraylib.a:
	$(MAKE) -C raylib/src
	cp raylib/src/libraylib.a $(BUILD_DIR)/libraylib.a

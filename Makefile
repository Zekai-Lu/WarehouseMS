CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Iinclude -Isrc

APP = build/warehousems.exe
TEST = build/test_inventory.exe

CORE = src/inventory.c src/storage.c

all: $(APP)

$(APP): $(CORE) src/main.c
	mkdir -p build
	$(CC) $(CFLAGS) $(CORE) src/main.c -o $(APP)

$(TEST): $(CORE) tests/test_inventory.c
	mkdir -p build
	$(CC) $(CFLAGS) $(CORE) tests/test_inventory.c -o $(TEST)

test: $(TEST)
	./$(TEST)

clean:
	rm -f build/warehousems.exe build/test_inventory.exe

.PHONY: all test clean
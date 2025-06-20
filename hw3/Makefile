CFLAGS := -O2 -Wall -fsanitize=address -fsanitize=undefined -Wextra -fstack-protector-all -ggdb
all : vmfs
	./vmfs

vmfs: vmfs.cpp
	g++ -o $@ $^ $(CFLAGS)

clean:
	rm -f vmfs disk

.PHONY: all clean

CFLAGS:= -O2 -Wall -fsanitize=address -fsanitize=undefined -Wextra -fstack-protector-all -ggdb

all : P-C
	./P-C

P-C:P-C.cpp
	g++ -o $@ $< -std=c++20 $(CFLAGS)

clear:
	rm -f P-C *.out


.PHONY: all clear
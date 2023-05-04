CFLAGS=-Wall -Wextra -Werror -O3 
TARGETS=lab12knmN32511 libknmN32511.so

.PHONY: all clean

all: $(TARGETS)

clean:
	rm -rf *.o $(TARGETS)

lab12knmN32511: lab12knmN32511.c plugin_api.h
	gcc $(CFLAGS) -o lab12knmN32511 lab12knmN32511.c -ldl

libknmN32511.so: lib12knmN32511.c plugin_api.h
	gcc $(CFLAGS) -shared -fPIC -o libknmN32511.so lib12knmN32511.c -ldl -lm




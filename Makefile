# Flags
CFLAGS=-Wall -Werror

build:
	g++ $(CFLAGS) -o tema1 tema1.cpp -lpthread

build_debug:
	g++ $(CFLAGS) -o tema1 tema1.cpp -lpthread -O0 -g3 -DDEBUG

clean:
	rm -f tema1

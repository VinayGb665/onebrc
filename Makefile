clean:
	rm -rf onebrc.o

build:
	cc -O0 main.c -o onebrc.o

run:
	time ./onebrc.o

all: build run

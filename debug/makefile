ccFlags := -lOpenCL
OS := $(shell uname)
ifeq ($(OS), Darwin)
ccFlags = -framework OpenCL
endif

clDebug: main.o
	g++ -w -o clDebug main.o $(ccFlags)

main.o: clBuildDebug.c
	g++ -w -c -o main.o clBuildDebug.c

clean:
	rm clDebug main.o

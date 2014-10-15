o := build
ccFlags := -lOpenCL
OS := $(shell uname)
ifeq ($(OS), Darwin)
ccFlags = -framework OpenCL -framework OpenGL -framework GLUT
endif

main: $(o)/main.o
	g++ -w -o main $(o)/main.o $(ccFlags)

$(o)/main.o: main.cpp
	g++ -w -c -o $(o)/main.o main.cpp

clean:
	rm main $(o)/main.o

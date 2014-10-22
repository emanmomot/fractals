o := build
ccFlags := -lOpenCL -lGL -lGLU -lglut -lGLEW
OS := $(shell uname)
ifeq ($(OS), Darwin)
ccFlags = -framework OpenCL -framework OpenGL -framework GLUT
endif

main: $(o)/main.o $(o)/Camera.o $(o)/Input.o
	g++ -w -o main $(o)/main.o $(o)/Camera.o $(o)/Input.o $(ccFlags)

$(o)/main.o: main.cpp Camera.h
	g++ -w -c -o $(o)/main.o main.cpp
	
$(o)/Camera.o: Camera.cpp Camera.h
	g++ -w -c -o $(o)/Camera.o Camera.cpp
	
$(o)/Input.o: Input.cpp Input.h Camera.h
	g++ -w -c -o $(o)/Input.o Input.cpp

clean:
	rm main $(o)/*.o

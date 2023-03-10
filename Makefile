ifeq ($(OS),Windows_NT)
  LDFLAGS = -l:libglfw3.a -l:libGLEW32.a -l:libopengl32.a -l:libgdi32.a -DGLEW_STATIC
else
  LDFLAGS = -lglfw -lGLEW -lGL
endif

all: main.c main2.c linmath.h
	gcc -o sierpinski -Wall main.c $(LDFLAGS)
	gcc -o sierpinski2 -Wall main2.c $(LDFLAGS)

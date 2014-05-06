all: mp2

clean:
	rm -f mp2

mp2: mp2.cc shader.cc mountain-retained.cpp
	g++ `pkg-config --cflags --libs glew glfw3` -framework opengl shader.cc mountain-retained.cpp mp2.cc -o mp2

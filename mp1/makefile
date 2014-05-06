all: mp1

clean:
	rm -f mp1

mp1: mp1.cc shader.cc
	g++ `pkg-config --libs glfw3 glew` -framework opengl mp1.cc shader.cc -o mp1

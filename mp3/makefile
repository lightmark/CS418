all: mp3

clean:
	rm -f mp3

mp3: mp3.cc shader.cc
	g++ `pkg-config --cflags --libs glew glfw3` -framework opengl -lsoil shader.cc mp3.cc -o mp3

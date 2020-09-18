CXX = g++
CXXFLAGS = -Wall -g -std=gnu++11 #-std=c++11 this flag doesn't work with CImg.h

# fun fact: extra spaces after the end of your list will result in circular dependencies
# https://stackoverflow.com/questions/33182500/makefile-circular-dependency-error
SRC = main

OBJ = $(SRC).o shader.o Particle.o System_NP.o stb_image.o #System.o
DEPS =

# What worked was using freeglut.dll & libfreeglut.a the 32 bit version
# GLEW library must be listed/included before GLUT
LDIR = -L.\lib -L\MinGW\glew-2.1.0\lib\Release\Win32 -L\MinGW\glfw-3.3.bin.WIN32\lib-mingw
LIBS = -lglew32s -lglfw3 -lopengl32 -lglu32 -lgdi32 

IDIR = -I\MinGW\glew-2.1.0\include -I\MinGW\glfw-3.3.bin.WIN32\include -I\MinGW\glm

all: $(SRC)

%.o : %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) $(IDIR) -c $< -o $@

$(SRC): $(OBJ)
	$(CXX) $(CXXFLAGS) $(IDIR) -o $@ $^ $(LDIR) $(LIBS)

clean:
	rm -f $(OBJS) main $(OBJ)

CC=g++
CFLAGS=-Wall -IInclude
LDFLAGS=
EXEC=GPU-filter
LIBS=-lglfw -lGL -lGLU -lm -lglut -lGLEW

all: $(EXEC)

GPU-filter: GPU-filter.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

GPU-filter.o: GPU-filter.h

%.o: %.cpp
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)
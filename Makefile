CC = gcc
CFLAGS = -g -Wall
SRC = project2.c student2.c
HEADERS = project2.h
TARGET = project2

all: $(TARGET)

$(TARGET): $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) *.o

run: $(TARGET)
	./$(TARGET) 100 0.1 0.2 0.0 2 1000 1



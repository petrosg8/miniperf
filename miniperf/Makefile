CC = gcc
CFLAGS = -Wall -pthread

TARGET = miniperf

SRCS = main.c server.c client.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)

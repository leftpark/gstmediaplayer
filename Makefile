CC = gcc
CFLAGS = `pkg-config gstreamer-1.0 --cflags`
LIBS = `pkg-config gstreamer-1.0 --libs`
TARGET = gstmediaplayer

gstmediaplayer: main.c player.c scanner.c
	$(CC) $(CFLAGS) main.c player.c scanner.c -o $(TARGET) $(LIBS) 

clean:
	rm $(TARGET)

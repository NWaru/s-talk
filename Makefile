

CFLAGS = -Wall -Werror -std=c11 -D_POSIX_C_SOURCE=200809L -g

all: build

build:
	gcc $(CFLAGS) s-talk.c receiver.c sender.c writer.c reader.c shutdownManager.c instructorList.o -pthread -o s-talk

clean:
	rm -f s-talk


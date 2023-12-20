CC = gcc
CFLAGS = -Wall -Wextra -std=c11

uhttpd: http.c main.c
	$(CC) $(CFLAGS) http.c main.c -o uhttpd

clean:
	rm -f uhttpd

# We assume clang/gcc
CFLAGS = -Wall -Wextra -Werror -O2 -g3
#CFLAGS += -fsanitize=address

all: fizzbuzzvm fizzbuzzreg

fizzbuzzvm: fizzbuzzvm.c

fizzbuzzreg: fizzbuzzreg.c

clean:
	rm fizzbuzzvm fizzbuzzreg

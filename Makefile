jlisp: jlisp.c mpc.c
	cc -std=c99 -Wall jlisp.c mpc.c -ledit -lm -o jlisp


PHONY = clean
clean:
	@rm -f jlisp

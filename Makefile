CC=cc
C_FILES=$(wildcard src/*.c)

.PHONY: posipakr

posipakr: ${C_FILES	
	@$(CC) ${C_FILES} -o $@
	@strip $@

clean:
	@rm -f posipakr
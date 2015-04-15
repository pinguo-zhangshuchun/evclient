PROJ = evclient
CC = gcc
CFLAGS =  -I. \
		  -g 
	      #-DDEBUG #-m32

SRC = $(wildcard *.c) $(wildcard src/*.c)
OBJ = $(patsubst %.c, %.o, $(SRC))

$(PROJ) : $(OBJ)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

.PHONEY:clean $(PROJ) 
clean:
	@rm -rvf $(PROJ) $(OBJ)

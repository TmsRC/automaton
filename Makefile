MF=	Makefile

CC=	mpicc
#CC=	gcc
CFLAGS=	-O3 -Wall -g

LFLAGS= $(CFLAGS)

EXE=	automaton

INC= \
	automaton.h \
	arralloc.h

SRC= \
	automaton.c \
	functions.c \
	cellio.c \
	unirand.c \
	arralloc.c \
	mpi_functionalities.c \
	tests.c
#
# No need to edit below this line
#

.SUFFIXES:
.SUFFIXES: .c .o

OBJ=	$(SRC:.c=.o)

.c.o:
	$(CC) $(CFLAGS) -c $<

all:	$(EXE)

$(OBJ):	$(INC)

$(EXE):	$(OBJ)
	$(CC) $(LFLAGS) -o $@ $(OBJ)

$(OBJ):	$(MF)

clean:
	rm -f $(EXE) $(OBJ) core

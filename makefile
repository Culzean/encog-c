IDIR =./
CC=gcc
CFLAGS=-I$(IDIR) -fopenmp -std=c99 -pedantic -Wall

ODIR=obj
LDIR =./lib

LIBS=-lm

_DEPS = encog.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = activation.o data.o  errorcalc.o example.o network.o pso.o util.o vector.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

encog: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 

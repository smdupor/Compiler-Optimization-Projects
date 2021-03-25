
.SUFFIXES: .o .c .cpp .lex .yy .lex.c .yy.c

%.o: %.c
	$(CC) -Wall -Wno-unused-function -Werror -g -O0 -c $(INCLUDE) $(CFLAGS) $(DEFS) $< -o $@

%.lex.c: %.lex
	$(LEX) -o$@ $<

%.cpp: %.lex
	$(LEX) -+ -o$@ $<

%.y.c: %.y
	$(YACC) --verbose -d -o$@ $<


LEX = flex
YACC = bison
CC  = gcc
CXX = g++
LD  = gcc

CFLAGS = 
LDFLAGS =-lm
DEFINES = 
INCLUDE = 

LIBS =


.PHONY: all tests

OBJS :=  main.o s3.y.o s3.lex.o 

all: opt

opt: $(OBJS)	
	$(LD) -o $@ $(LDFLAGS) $(LIBS) $^

clean:
	rm -Rf *.o *.output *~ opt s3.y.h
MCC = ../mcc

.SUFFIXES: %.o %.c

CFLAGS =

ifdef VERBOSE
CFLAGS += -v
endif

tests :=  \
	test_000.asm \
	test_001.asm \
	test_002.asm \
	test_003.asm \
	test_004.asm \
	test_005.asm \
	test_006.asm \
	test_007.asm \
	test_008.asm \
	test_009.asm \
	test_010.asm \
	test_011.asm \
	test_012.asm \
	test_013.asm \
	test_014.asm \


%.asm: %.c
ifdef DEBUG
	gdb --args $(MCC) $(CFLAGS) -o $@ $<
else
	$(MCC) $(CFLAGS) -o $@ $<
endif

all: clean $(tests)

clean:
	rm -Rf *.asm *.o *~ *.sym

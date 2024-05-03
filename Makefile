SDIR= src
ODIR= output
sources := $(wildcard $(SDIR)/hw*.c)
targets := $(patsubst $(SDIR)/hw%.c, $(ODIR)/hw2%.output,$(sources))

CC= gcc
CFLAGS= -O3 -std=c11 -Wall

$(SDIR)/hw2%.o: $(SDIR)/hw%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(ODIR)/%.output: $(SDIR)/%.o
	$(CC) $(CFLAGS) -o $@ $^ -lm

all: check $(targets)

check:
	mkdir -p $(ODIR)

.PHONY: check all clean

clean:
	rm -f $(SDIR)/*.o $(ODIR)/*.output

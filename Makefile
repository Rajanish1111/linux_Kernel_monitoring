CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lncurses

PROG = slab-watcher

all: $(PROG)

$(PROG): $(PROG).c
# IMPORTANT: This next line MUST begin with a single TAB, not spaces.
	$(CC) $(CFLAGS) -o $(PROG) $(PROG).c $(LDFLAGS)

clean:
# IMPORTANT: This next line MUST also begin with a single TAB.
	rm -f $(PROG)

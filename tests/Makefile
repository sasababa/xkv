targets := fault_test

.PHONY: all
all: $(targets)

fault_test: fault_test.c
	$(CC) -O0 -o fault_test -lrt fault_test.c

.PHONY: clean
clean:
	rm -f $(targets)


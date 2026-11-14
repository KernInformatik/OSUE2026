include config.mk

.PHONY: options all clean doc
options:
	@echo "build options:"
	@echo "CFLAGS = $(CFLAGS)"
	@echo "CC     = $(CC)"

all: $(MAIN)

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(MAIN): $(MAIN_OBJ)
	$(CC) $(CFLAGS) -o $@ $(MAIN_OBJ)  -Wl,-rpath,'$$ORIGIN' -pthread -lrt

doc:
	doxygen Doxyfile

clean:
	rm -rf *.o $(MAIN) *.so
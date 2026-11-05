include config.mk

.PHONY: options all clean doc
options:
	@echo "build options:"
	@echo "CFLAGS = $(CFLAGS)"
	@echo "CC     = $(CC)"

all: lib $(MAIN)

lib: $(LIB_SEM_INTF)

$(LIB_OBJS):$(LIB_SRCS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -c $< -o $@

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB_SEM_INTF): $(LIB_OBJS)
	$(CC) $(CFLAGS) -shared -o $@ $^ -pthread -lrt

$(MAIN): $(MAIN_OBJ) $(LIB_OBJS)
	$(CC) $(CFLAGS) -o $@ $(MAIN_OBJ) -L. -lsemaphore_interface -Wl,-rpath,'$$ORIGIN' -pthread -lrt

doc:
	doxygen Doxyfile

clean:
	rm -rf *.o $(GENERATOR) $(SUPERVISOR) *.so
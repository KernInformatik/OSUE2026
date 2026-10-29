# Src: https://stackoverflow.com/questions/30573481/how-to-write-a-makefile-with-separate-source-and-header-directories
#https://wiki.osdev.org/Makefile
include config.mk

.PHONY: options all lib clean doc
options:
	@echo "build options:"
	@echo "CFLAGS = $(CFLAGS)"
	@echo "CC     = $(CC)"

all: lib $(GENERATOR) $(SUPERVISOR)

lib: $(LIB_SEM_INTF)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/lib/%.o: $(LIB_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -c $< -o $@

# library objects need -fPIC and -c
$(BIN_DIR)/lib/%.o: $(LIB_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -c $< -o $@


# For the static library
#https://tldp.org/HOWTO/Program-Library-HOWTO/static-libraries.html
#And for the @D
#https://stackoverflow.com/questions/71181638/trying-compile-with-makefile-but-say-mkdir-command-not-found
$(LIB_SEM_INTF): $(LIB_OBJS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -shared -o $@ $^ -pthread -lrt

$(GENERATOR): $(GENERATOR_OBJ) $(LIB_SEM_INTF)
	$(CC) $(CFLAGS) -o $@ $(GENERATOR_OBJ) -L$(BIN_DIR) -lsemaphore_interface -Wl,-rpath,'$$ORIGIN' -pthread -lrt

$(SUPERVISOR): $(SUPERVISOR_OBJ) $(LIB_SEM_INTF)
	$(CC) $(CFLAGS) -o $@ $(SUPERVISOR_OBJ) -L$(BIN_DIR) -lsemaphore_interface -Wl,-rpath,'$$ORIGIN' -pthread -lrt


clean:
	rm -rf $(BIN_DIR)

doc:
	doxygen Doxyfile
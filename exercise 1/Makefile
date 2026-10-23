# Src: https://stackoverflow.com/questions/30573481/how-to-write-a-makefile-with-separate-source-and-header-directories
#https://wiki.osdev.org/Makefile
include config.mk

.PHONY: options all clean doc

options:
	@echo "build options:"
	@echo "CFLAGS = $(CFLAGS)"
	@echo "CC     = $(CC)"

all: $(LIB_SEM_INTF) $(GENERATOR) $(SUPERVISOR)

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/lib/%.o: $(LIB_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@


# For the static library
#https://tldp.org/HOWTO/Program-Library-HOWTO/static-libraries.html
#And for the @D
#https://stackoverflow.com/questions/71181638/trying-compile-with-makefile-but-say-mkdir-command-not-found
$(LIB_SEM_INTF): $(LIB_OBJS)
	@mkdir -p $(@D)
	ar rcs $@ $^

$(GENERATOR): $(GENERATOR_OBJ) $(LIB_SEM_INTF)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $(GENERATOR_OBJ) $(LIB_SEM_INTF)

$(SUPERVISOR): $(SUPERVISOR_OBJ) $(LIB_SEM_INTF)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $(SUPERVISOR_OBJ) $(LIB_SEM_INTF)

clean:
	rm -rf $(BIN_DIR) $(GENERATOR) $(SUPERVISOR)

doc:
	doxygen Doxyfile
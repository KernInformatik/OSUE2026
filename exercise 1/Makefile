include config.mk

.PHONY: options all lib clean doc
options:
	@echo "build options:"
	@echo "CFLAGS = $(CFLAGS)"
	@echo "CC     = $(CC)"

all: lib $(GENERATOR) $(SUPERVISOR)

lib: $(LIB_SEM_INTF)

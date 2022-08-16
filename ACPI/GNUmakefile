override ACPI := acpi.elf

define DEFAULT_VAR =
    ifeq ($(origin $1), default)
        override $(1) := $(2)
    endif
    ifeq ($(origin $1), undefined)
        override $(1) := $(2)
    endif
endef

$(eval $(call DEFAULT_VAR,CC,cc))


CFLAGS ?= -Og -g -Wall -Wextra -Wpedantic -pipe -I ../include



override INTERNALCFLAGS := \
	-std=c11               \
	-ffreestanding         \
	-fno-stack-protector   \
	-fno-stack-check       \
	-fno-pie               \
	-fno-pic               \
	-m64                   \
	-march=x86-64          \
	-mabi=sysv             \
	-mno-80387             \
	-mno-mmx               \
	-mno-sse               \
	-mno-sse2              \
	-mno-red-zone          \
	-mcmodel=kernel        \
	-MMD

override CFILES := $(shell find ./ -type f -name '*.c')
override ASFILES := $(shell find ./ -type f -name '*.S')
override NASMFILES := $(shell find ./ -type f -name '*.asm')
override OBJ := $(CFILES:.c=.o) $(ASFILES:.S=.o) $(NASMFILES:.asm=.o)
override HEADER_DEPS := $(CFILES:.c=.d) $(ASFILES:.S=.d)


.PHONY: all
all: $(ACPI)

# Link rules for the final kernel executable.
$(ACPI): $(OBJ)
	$(CC) -c $(CFLAGS) $(INTERNALCFLAGS) $(OBJ) -o $@


# Include header dependencies.
-include $(HEADER_DEPS)

# Compilation rules for *.c files.
%.o: %.c
	$(CC) $(CFLAGS) $(INTERNALCFLAGS) -c $< -o $@

# Compilation rules for *.S files.
%.o: %.S
	$(CC) $(CFLAGS) $(CFLAGS) $(INTERNALCFLAGS) -c $< -o $@

# Compilation rules for *.asm (nasm) files.
%.o: %.asm
	nasm $(NASMFLAGS) $(INTERNALNASMFLAGS) $< -o $@

# Remove object files and the final executable.
.PHONY: clean
clean:
	rm -rf $(KERNEL) $(OBJ) $(HEADER_DEPS)

# Makefile for FreeRTOS demo
#
CONFIG_FILE=.config
CONFIGS_DIR=configs
$(shell if [ ! -e $(CONFIG_FILE) ]; then \
		touch $(CONFIG_FILE); \
		make defaultconfig > $(CONFIG_FILE); \
		fi)
include $(CONFIG_FILE)

TOP = $(PWD)

CROSS_COMPILE ?= mips-img-elf
CFLAGS += -mabi=64
GCC = $(CROSS_COMPILE)-gcc
CFLAGS += -EL
CFLAGS += -msoft-float

# by default we try to enable GIC EIC if possible, if you want to force it to be
# off set NO_GIC_EIC=1
ifeq ($(NO_GIC_EIC),1)
CFLAGS += -DCONFIG_NO_GIC_EIC
endif

MIPS64_SRC_DIR=../../Source/portable/GCC/MIPS64

# Include paths
INCLUDE = -I. -I./include -I../../Source/include -I$(MIPS64_SRC_DIR) -I../Common/include

# C Flags
ifeq ($(DEBUG),1)
CFLAGS += -march=$(ARCH) -g $(INCLUDE) -O0
else
CFLAGS += -march=$(ARCH) -g $(INCLUDE) -O3
endif

# Customise FreeRTOS settings
CFLAGS += -DCONFIG_CPU_CLOCK_HZ=$(CPU_CLOCK_HZ)
CFLAGS += -DCONFIG_TICK_RATE_HZ=$(TICK_RATE_HZ)
CFLAGS += -DCONFIG_TIMER_CLOCK_HZ=$(TIMER_CLOCK_HZ)

CFLAGS += -DCONFIG_STACK_SIZE=$(STACK_SIZE)
CFLAGS += -DCONFIG_HEAP_SIZE=$(HEAP_SIZE)
CFLAGS += -DCONFIG_MAX_PRIORITIES=$(MAX_PRIORITIES)

# Use uhi64_64.ld from SDK. 
LDSCRIPT=-Tuhi64_64.ld

# Set vector count to maximum possible (64) when eic is selected.
# This avoids the potential problem of overwriting text section when installing
# an isr due to overlapping.
LDSCRIPT+=-Wl,--defsym,__isr_vec_count=64

LDSCRIPT += -lcm3

# __use_excpt_boot means we can use a bootloader (ie: uboot) to perform uhi
# functionality. We can't assume such a bootloader exist so always set it to 0.
LDSCRIPT+=-Wl,--defsym,__use_excpt_boot=0

# some sensible defaults for memory setup and application start
LDSCRIPT+=-Wl,--defsym,__app_start=$(APP_START)
LDSCRIPT+=-Wl,--defsym,__memory_size=$(MEMORY_SIZE)
LDSCRIPT+=-Wl,--defsym,__memory_base=$(MEMORY_BASE)

# Source & object files
SRC := \
	../../Source/queue.c \
	../../Source/tasks.c \
	../../Source/list.c \
	../../Source/timers.c \
	$(MIPS64_SRC_DIR)/port.c \
	$(MIPS64_SRC_DIR)/int_handler.c \
	$(MIPS64_SRC_DIR)/port_timer.c \
	$(MIPS64_SRC_DIR)/init_hook.c \
	../../Source/portable/MemMang/heap_1.c \
	../../Source/event_groups.c \
	main.c

ASMSRC := \
	$(MIPS64_SRC_DIR)/port_asm.S \
	$(MIPS64_SRC_DIR)/handler.S

OBJDIR ?= build

BOBJ := $(BOOTSRC:.c=.o) $(BOOTASMSRC:.S=.o)
TOBJ := $(SRC:.c=.o) $(ASMSRC:.S=.o)
TOBJ += $(BOBJ)

TARGET_OBJ := $(addprefix $(OBJDIR)/, $(notdir $(TOBJ)))
BOOT_OBJ := $(addprefix $(OBJDIR)/, $(notdir $(BOBJ)))

# Rules
TARGET = RTOSDemo.elf

CONFIGS=$(shell cd $(CONFIGS_DIR); ls *)

.PHONY: all clean $(CONFIGS)
all: $(TARGET) $(TARGET_BOOT)

$(TARGET): $(TARGET_OBJ)
	$(GCC) $(CFLAGS) -Wl,-Map=$(subst .elf,.map,$@) $(LDSCRIPT) -o $@ $^

$(TARGET_OBJ): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o : ../Common/Minimal/%.c $(CONFIG_FILE)
	$(GCC) -c $(CFLAGS) $< -o $@
	@$(GCC) -c $(CFLAGS) -MM -MT $@ -MF $(subst .o,.d,$@) $< -o $(subst .o,.d,$@)

$(OBJDIR)/%.o : ../../Source/portable/MemMang/%.c $(CONFIG_FILE)
	$(GCC) -c $(CFLAGS) $< -o $@
	@$(GCC) -c $(CFLAGS) -MM -MT $@ -MF $(subst .o,.d,$@) $< -o $(subst .o,.d,$@)

$(OBJDIR)/%.o : $(MIPS64_SRC_DIR)/%.c $(CONFIG_FILE)
	$(GCC) -c $(CFLAGS) $< -o $@
	@$(GCC) -c $(CFLAGS) -MM -MT $@ -MF $(subst .o,.d,$@) $< -o $(subst .o,.d,$@)

$(OBJDIR)/%.o : ../../Source/%.c $(CONFIG_FILE)
	$(GCC) -c $(CFLAGS) $< -o $@
	@$(GCC) -c $(CFLAGS) -MM -MT $@ -MF $(subst .o,.d,$@) $< -o $(subst .o,.d,$@)

$(OBJDIR)/%.o : %.c $(CONFIG_FILE)
	$(GCC) -c $(CFLAGS) $< -o $@
	@$(GCC) -c $(CFLAGS) -MM -MT $@ -MF $(subst .o,.d,$@) $< -o $(subst .o,.d,$@)

$(OBJDIR)/%.o : $(MIPS64_SRC_DIR)/%.S $(CONFIG_FILE)
	$(GCC) -c $(CFLAGS) $< -o $@
	@$(GCC) -c $(CFLAGS) -MM -MT $@ -MF $(subst .o,.d,$@) $< -o $(subst .o,.d,$@)

$(OBJDIR)/%.o : boot/%.c $(CONFIG_FILE)
	$(GCC) -c $(CFLAGS) $< -o $@
	@$(GCC) -c $(CFLAGS) -MM -MT $@ -MF $(subst .o,.d,$@) $< -o $(subst .o,.d,$@)

$(OBJDIR)/%.o : boot/%.S $(CONFIG_FILE)
	$(GCC) -c $(CFLAGS) $< -o $@
	@$(GCC) -c $(CFLAGS) -MM -MT $@ -MF $(subst .o,.d,$@) $< -o $(subst .o,.d,$@)

$(OBJDIR)/%.o : %.S $(CONFIG_FILE)
	$(GCC) -c $(CFLAGS) $< -o $@
	@$(GCC) -c $(CFLAGS) -MM -MT $@ -MF $(subst .o,.d,$@) $< -o $(subst .o,.d,$@)

$(OBJDIR)/%.o: $(MIPS_ELF_ROOT)/share/mips/boot/%.S $(CONFIG_FILE)
	$(GCC) -c $(CFLAGS) $< -o $@
	@$(GCC) -c $(CFLAGS) -MM -MT $@ -MF $(subst .o,.d,$@) $< -o $(subst .o,.d,$@)

# We need to modify reset_predef.S to support booting in EVA mode but we don't
# want to make the source file part of our source as it has different copyright
# header. Copy it at runtime and patch it to avoid redistributing it.
boot/reset_predef.S: $(MIPS_ELF_ROOT)/share/mips/boot/reset_predef.S
	@cp $^ $@
	@patch -p0 < boot/reset_predef.patch

clean:
	rm -rf build
	rm -f $(TARGET) $(subst .elf,.map,$(TARGET))
ifeq ($(TWO_STAGE_BOOT), 1)
	rm -f $(TARGET_BOOT) $(subst .elf,.map,$(TARGET_BOOT))
endif

$(CONFIGS):
	@cat $(CONFIGS_DIR)/$@ > $(CONFIG_FILE)

-include $(TARGET_OBJ:.o=.d)
-include $(BOOT_OBJ:.o=.d)

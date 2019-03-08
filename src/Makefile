PROJECT = ir-remote
BUILD_DIR = bin

SHARED_DIR = ../common
CFILES = main.c
CFILES += nec.c

DEVICE=stm32f051r8
OOCD_FILE = board/stm32f0discovery.cfg

# You shouldn't have to edit anything below here.
VPATH += $(SHARED_DIR)
INCLUDES += $(patsubst %,-I%, . $(SHARED_DIR))
OPENCM3_DIR=../libopencm3

include $(OPENCM3_DIR)/mk/genlink-config.mk
include ../rules.mk
include $(OPENCM3_DIR)/mk/genlink-rules.mk

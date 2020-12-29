# This is a special makefile format for compiling kernel drivers
ifeq ($(shell uname -m),ppc64)
EXTRA_CFLAGS += -DACL_BIG_ENDIAN
endif

EXTRA_CFLAGS += -I$(PWD)/../../include
EXTRA_CFLAGS += -I$(PWD)/../../../../$(BSP_NAME)/host/include

ifneq ($(BSP_NAME),)
MODULENAME ?= aclpci_$(BSP_NAME)_drv
EXTRA_CFLAGS += -DBSP_NAME=$(BSP_NAME)
else
MODULENAME ?= aclpci_drv
endif

# Final module
obj-m := $(MODULENAME).o

# List of object files to compile for the final module.
$(MODULENAME)-y := aclpci_queue.o aclpci.o aclpci_fileio.o aclpci_dma.o aclpci_pr.o aclpci_cmd.o

clean:
	$(RM) *.o *.ko *.mod.* *.mod .*.cmd module*.order *.*ymvers

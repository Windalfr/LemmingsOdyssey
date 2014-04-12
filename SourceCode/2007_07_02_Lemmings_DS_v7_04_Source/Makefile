#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM)
endif

include $(DEVKITARM)/ds_rules

export TARGET			:=	lemmingsds
export TOPDIR			:=	$(CURDIR)

#---------------------------------------------------------------------------------
# path to tools - this can be deleted if you set the path in windows
#---------------------------------------------------------------------------------


.PHONY: $(TARGET).arm7 $(TARGET).arm9

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all: final_output/$(TARGET)_scsd.nds

final_output/$(TARGET)_scsd.nds	: $(TARGET)_scsd.nds
# Make final_output directory
	mkdir -p final_output
# Copy everything from output_tree to final_output
	cp -pRv output_tree/* final_output/
# Copy the prepared scsd.
	cp $(TARGET)_scsd.nds final_output/$(TARGET)_scsd.nds

$(TARGET)_scsd.nds	: $(TARGET).nds
# Copy the rom file as scsd
	cp $(TARGET).nds $(TARGET)_scsd.nds
# Patch the scsd file.
	dlditool $(DEVKITPRO)/dldi/scsd.dldi $(TARGET)_scsd.nds

#---------------------------------------------------------------------------------
$(TARGET).nds	:	$(TARGET).arm7 $(TARGET).arm9
	ndstool	-c $(TARGET).nds -7 $(TARGET).arm7 -9 $(TARGET).arm9

#---------------------------------------------------------------------------------
$(TARGET).arm7	: arm7/$(TARGET).elf
$(TARGET).arm9	: arm9/$(TARGET).elf

#---------------------------------------------------------------------------------
arm7/$(TARGET).elf:
	make -C arm7
	
#---------------------------------------------------------------------------------
arm9/$(TARGET).elf:	
	make -C arm9

#---------------------------------------------------------------------------------
clean:
	make -C arm9 clean
	make -C arm7 clean
	rm -f  $(TARGET).nds 
	rm -f  $(TARGET)_scsd.nds
	rm -f  $(TARGET).arm7
	rm -f  $(TARGET).arm9
	rm -rf final_output/*
	rm -rf final_output/*.*

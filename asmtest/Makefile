
ASM=wla-z80
#ASM=z80asm
PROGNAME=bitmap
ASMFLAGS=-v 
MEKASTATE=~/SegaSC3000/Meka/meka/meka/Saves/SegaBASICLevel3.S00
# -I.

all: $(PROGNAME).bin $(PROGNAME).bdbin $(MEKASTATE)

%.o: %.asm
	$(ASM) $(ASMFLAGS) -o $@ $< 	

$(PROGNAME).bin: $(PROGNAME).o
	echo [objects]>wlalinkfile
	echo $(PROGNAME).o>>wlalinkfile
	wlalink -b wlalinkfile $@

$(PROGNAME).bdbin: $(PROGNAME).bin
	sc3ktape basicinit.sc.bas -tobdbin -o $(PROGNAME).bdbin -DBINOBJ=$(PROGNAME).bin
$(PROGNAME).wav: $(PROGNAME).bin	
	sc3ktape basicinit.sc.bas -o $(PROGNAME).wav -DBINOBJ=$(PROGNAME).bin

$(MEKASTATE): $(PROGNAME).bdbin
	# copy bdbin inside Meka emu's state in a basiscLevelIII saved state.
	filepatch $@ origstate.S00 $(PROGNAME).bdbin 6257
	

# ~/SegaSC3000/Meka/meka/meka/Saves/Sega\ BASIC\ Level\ 3\ V1\ \(SC-3000\).S00
clean: 
	rm -rf *.bin *.o *.bdbin wlalinkfile
#
# bitmap.bin: bitmap.o
#	wlalink -v bitmap.o

#    bitmap.o: bitmap.asm
#        wla-z80 -o main.bin main.asm

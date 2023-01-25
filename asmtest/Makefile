
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
	#OK Meka Linux:
#	cat $(PROGNAME).bdbin | dd of=origstateLINUX.S00 bs=1 seek=6257 conv=notrunc
#	cp origstateLINUX.S00 ~/SegaSC3000/Meka/meka/meka/Saves/Sega_BASIC_Level_3_V1_SC-3000.S00
	mekastateloader ~/SegaSC3000/Meka/meka/meka/Saves/SegaBASICLevel3.S00 $(PROGNAME).bdbin

	#cat $(PROGNAME).bdbin | dd of=origstate.S00 bs=1 seek=3517 conv=notrunc
	#cp origstate.S00 ~/SegaSC3000/Meka/meka/meka/Saves/SegaBASICLevel3.S00
	#OK Meka Windows	
	#cat $(PROGNAME).bdbin | dd of=origstate.S00 bs=1 seek=6253 conv=notrunc
	#cp origstate.S00 /c/Users/victo/Documents/segaa/mekaw-2019-12-13/Saves/Sega_BASIC_Level_3_V1_\(SC-3000\).S00

clean: 
	rm -rf *.bin *.o *.bdbin wlalinkfile
#
# bitmap.bin: bitmap.o
#	wlalink -v bitmap.o

#    bitmap.o: bitmap.asm
#        wla-z80 -o main.bin main.asm

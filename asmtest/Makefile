
# ASMZ=wla-z80
ASM=z80asm
PROGNAME=bitmap
ASMFLAGS=-v 
MEKASTATE=~/SegaSC3000/Meka/meka/meka/Saves/SegaBASICLevel3.S00
# -I.
%.o: %.asm
	$(ASM) $(ASMFLAGS) -o $@ $< 	
	
all: $(PROGNAME).o $(PROGNAME).bdbin $(MEKASTATE)

$(PROGNAME).bdbin: $(PROGNAME).o
	sc3ktape basicinit.sc.bas -tobdbin -o $(PROGNAME).bdbin -DBINOBJ=$(PROGNAME).o

$(MEKASTATE): $(PROGNAME).bdbin
	filepatch $@ origstate.S00 $(PROGNAME).bdbin 6257
	

# ~/SegaSC3000/Meka/meka/meka/Saves/Sega\ BASIC\ Level\ 3\ V1\ \(SC-3000\).S00
clean: 
	rm -rf *.o *.bdbin
#
# bitmap.bin: bitmap.o
#	wlalink -v bitmap.o

#    bitmap.o: bitmap.asm
#        wla-z80 -o main.bin main.asm


ASM=wla-z80

PROGNAME=bitmap
ASMFLAGS=-v 
MEKASTATE=~/SegaSC3000/Meka/meka/meka/Saves/SegaBASICLevel3.S00
BASICINIT=basicinit.sc.bas
# -I.

all: $(PROGNAME).bin $(PROGNAME).bdbin $(MEKASTATE)

%.o: %.asm basicinit.basicoffsets.i
	$(ASM) $(ASMFLAGS) -o $@ $<

%.basicoffsets.i: %.sc.bas
	sc3ktape $@ -tobasicoffsets -o $<

$(PROGNAME).bin: $(PROGNAME).o
	echo [objects]>wlalinkfile
	echo $(PROGNAME).o>>wlalinkfile
	wlalink -b wlalinkfile $@

$(PROGNAME).bdbin: $(PROGNAME).bin $(BASICINIT)
	sc3ktape $(BASICINIT) -tobdbin -o $< -DBINOBJ=$(PROGNAME).bin
$(PROGNAME).wav: $(PROGNAME).bin $(BASICINIT)
	sc3ktape $(BASICINIT) -o $(PROGNAME).wav -DBINOBJ=$(PROGNAME).bin

$(MEKASTATE): $(PROGNAME).bdbin
	# copy bdbin inside Meka emu's state in a basiscLevelIII saved state.
	mekastateloader $(MEKASTATE) $(PROGNAME).bdbin

clean: 
	rm -rf *.bin *.o *.bdbin wlalinkfile

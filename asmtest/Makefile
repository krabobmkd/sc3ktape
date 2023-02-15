
ASM=wla-z80

#PROGNAME=bitmap
PROGNAME=demo
#PSGSONG=SonicGreenHill.psg
PSGSONG=SpaceHarrier09Syura.psg
#PSGSONG=Ghostbusters.psg

#CHARSET=aeg_collection_03.charset
CHARSET=aeg_collection_11.charset
#CHARSET=craze_dreams.charset
# CHARSET=c7up.charset

ASMFLAGS=-v -D SONG_FILE=$(PSGSONG) -D CHARSET=$(CHARSET) -I.

.PRECIOUS: $(CHARSET)

ifdef OS
    MEKASTATE=/c/Users/victo/Documents/segaa/mekaw-2019-12-13/Saves/Sega_BASIC_Level_3_V1_\(SC-3000\).S00
else
   ifeq ($(shell uname), Linux)
    MEKASTATE=~/SegaSC3000/Meka/meka/meka/Saves/SegaBASICLevel3.S00
   endif
endif

BASICINIT=basicinit.sc.bas

ASMFILES=decomp.asm linescroll.asm starfield3d.asm

all: $(PROGNAME).bin $(PROGNAME).bdbin $(MEKASTATE)

%.o: %.asm basicinit.basicoffsets.i $(ASMFILES) $(PSGSONG) $(CHARSET)
	$(ASM) $(ASMFLAGS) -o $@ $<

%.basicoffsets.i: %.sc.bas 
	echo " * * * * BLABLA"
	sc3ktape $< -tobasicoffsets -o $@ 
	cat $@
	
$(PROGNAME).bin: $(PROGNAME).o $(PSGSONG)
	echo [objects]>wlalinkfile
	echo $(PROGNAME).o>>wlalinkfile
	wlalink -b -S wlalinkfile $@

$(PROGNAME).bdbin: $(PROGNAME).bin $(BASICINIT)
	sc3ktape $(BASICINIT) -tobdbin -o $@ -DBINOBJ=$(PROGNAME).bin
$(PROGNAME).wav: $(PROGNAME).bin $(BASICINIT)
	sc3ktape $(BASICINIT) -o $(PROGNAME).wav -DBINOBJ=$(PROGNAME).bin -waves16le

# - -  load program as cassette audio to machine, using linux USB audio card
play: play_$(PROGNAME)

play_%: %.wav
	aplay --device front:CARD=ICUSBAUDIO7D,DEV=0 $<

# - - insert program into meka emulator basic rom state file, to run with load state.
$(MEKASTATE): $(PROGNAME).bdbin
	# copy bdbin inside Meka emu's state in a basiscLevelIII saved state.
	mekastateloader $(MEKASTATE) $(PROGNAME).bdbin

# - - - charset

%.charset: charsets/%.64c
	# 64 chars ->512
	#dd skip=2 count=512 if=$< of=$@ bs=1
	c64charsettoascii $@ $< charsets/c7up.64c
# - - - - music things
%.psg: %.vgm
	java -jar /usr/local/share/java/PSGTool.jar $< $@  -framerate 50
	# -uncompressed
clean: 
	rm -rf *.bin *.o *.bdbin *.psg wlalinkfile *.charset *.sym *.basicoffsets.i

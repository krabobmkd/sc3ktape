# sc3ktape
Sega SC-3000 Tape Wave Conversion tool (C++)

Compile with C++14

# Sega SC-3000 / SK1100 Tape-to-basic-to-tape converter v0.9

usage:
 
sc3ktape file.wave -o basicfile.sc.bas  [options] convert wave to basic.

sc3ktape file.txt/.bas -o basicfile.wave [options] convert basic to wave.

sc3ktape file.bin -o file.wave [options] convert asm binary to wave.

tape to basic options:
 
 -jp:      for japan basic, use utf8 katakana, else european dieresis.

 -tolabel: replace lines number by tabs and generate labels (usable with b2t).

basic to tape options:

 -nNAME: give header prog file name for tape wave header.

Input waves can use any frequencies (22050Hz,44100Hz), must be mono.

Ouput waves are always 8bit/ 22050Hz, which is enough for the hardware.

 Still beta but ok to put on main because usable.
 no known bug for basic to wave and wave to basic conversions, but more programs has yet to be tested to make sure all opcodes are correctly translated.

# Interesting part 
 
 The Sega SC3000 8Bit computer (1983-1985) was the computer version of the SG1000 console which prefigure the Sega Master System, and has actually a quite well done BASIC available as a cartridge. The 3 Console SG1000, MarkII and MarkIII could also use it with the keyboard extension. And the interesting thing is that basic and assembler program could be loaded from a cassette player that was linked to the computer or console with a standard audio mini jack cable. So You can just directly plug your PC output jack to your Sega SC3000 cassette input, type LOAD into the basic, and play the program as wave from your PC. 
 
 This converter is used to translate text basic files as waves sounds, in the format used by Sega cassettes, and also translate waves from cassettes into text basic program.
 
  There were both a sound format for basic and assembler binaries, and both could use rom functions from the BASIC cartridge.
 
 Note The file extension choosed for Sega Basic text file on the PC side is ".sc.bas": it makes it directly highlightable from a lots of editor that already detect .bas files as Basic files (Kate, QtCreator,...).
  
 Note with -tolabel option, the lines of the original source are removed, replaced with tabs at lines start, and asm-like labels are added at the place where goto, gosub,restore would search them ("on gosub" syntax with multiple jump are managed)
 Line numbers are recomputed the correct way when program is translated back to wave sound format. 


 All this is validated with real European SC3000 and japanese MarkIII+SK1100 hardware at home. 

 Working Programs will feed the validation drawer.

 
 To be continued...
 
 

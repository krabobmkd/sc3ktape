
EXE=SC3KWaveManager

OBJS= main.cppo SC3KBasic.cppo SoundReader.cppo

all: $(EXE)

clean:
	delete all force $(EXE) $(OBJS)

$(EXE): $(OBJS)
	#slink FROM $(OBJS) TO $@ sc nd
	#Copy res/Catabasis.exe.info $@.info
	#note: ld wont work ... + it need a final space !!!
	gcc $(OBJS) -o $(EXE) -lstdc++ -noixemul

%.cppo: %.cpp SC3KWaveReader.h WaveReader.h
	gcc -c $< -o $@ -noixemul


# with ixemul and strip: 237860    with noixemul: 245608




﻿5 PATTERNC#&HD0,"7884B4A4A4B48478"
10 PN=&H7F:DIM X1(255),X2(255),X3(25S),W(255),TZ(255)
20 FLAG=0
30 SCREEN1,1:CLS
40 PRINT"Music Editor,     By B.Brown. ";CHR$(&HD0);" 84"
50 PRINT"--------   ---------"
60 PRINT:PRINT"Options"
70 PRINT "1 - Play memory area"
80 PR1NT "2 - Create music"
90 PRINT "3 - Edit music array"
100 PRINT:PRINT "Select desired option:"
110 AA$= INKEY$:IF AA$=""THEN GOTO 110
120 IF AA$="l" THEN GOTO 820
130 IF AA$="2" THEN GOTO 900
140 IF AA$="3" THEN GOTO 1000
150 GOTO 110

200 REM INPUT ROUTINES
210 B1$=" 10000000"
220 PRINT"Freq (118-3500) of tone #";ZB;" ";:INPUT FT :IF FT<118 OR FT>3500 THEN GOTO 220
230 BT=3840000/(32*FT)
240 DB=INT(BT+.5):GOSUB 430

250 B1$=LEFT$(B1$,4)+A1$
260 B2$=A2$
270 INPUT"Tone Level (1-15) ";TL
280 IF(TL<1)OR(TL>15)THEN 270
290 DB=TL:GOSUB 430
300 B4$="1001"
310 B3$=B4$+RIGHT$(A2$,4)
320 GOSUB 800
330 REM N1=Byte1,N2=Byte2,N3=Atten
340 GS$=81S:GOSUB670:N1=OB
350 GS$=B2$:GOSUB670:N2=OB
360 GS$=B3$:GOSUB670:N3=OB
370 PRINT"Desired rest period"
380 PRINT"before next note.";:INPUT ZC :RETURN

390 REM PLAY ROUTINE
400 OUT(PN),N3:OUT(PN),N1:OUT(PN),N2
410 FOR TP=1 TO ZC:NEXT
420 RETURN

430 REM DEC TO BIN
440 REM INPUT=DB,OUTPUT=Al$,A2$
450 FOR ZZ=1 TO 10 : AA(ZZ)=0 : NEXT ZZ
460 DB=INT(DB)
470 FOR T3=1 TO 10
480 T2=DB MOD 2
490 IF T2=1 THEN AA(T3)=1
500 DB=INT(DB/2)
510 NEXT T3
520 A1$ ="":A2$= "":FOR ZZ=1 TO 10
530 A1$=A1$+STR$(AA(ZZ)):NEXT ZZ
540 GOSUB 580:A1$=SB$
550 A2$="00"+LEFT$(A1$,6)
560 A1$=RIGHT$(A1$,4)
570 RETURN

580 SA$="" 
590 FOR S=1 TO LEN(A1$)
600 IF MID$(A1$,S,1)=" " THEN 620
610 SA$=SA$+MID$(A1S,S,1)
620 NEXT S:SB$="" 
630 FOR S=1 TO LEN(SA$) 
640 SB$=SB$+MID$(SA$,LEN(SA$)+1-S,1) 
650 NEXT S 
660 RETURN 

670 REM STRING TO DECIMAL 
680 REM INPUT=GS$,OUTPUT=OB 
690 OB=0 
700 IF MID$(GS$,1,1)="1"THEN OB=OB+128 
710 IF MID$(GS$,2,1)="1"THEN OB=OB+64 
720 IF MID$(GS$,3,1)="1"THEN OB=OB+32 
730 IF MID$(GS$,4,1)="1"THEN OB=OB+16 
740 IF MID$(GS$,5,1)="1"THEN OB=OB+8 
750 IF MID$(GS$,6,1)="1"THEN OB=OB+4 
760 IF MID$(GS$,7,1)="1"THEN OB=OB+2 
770 IF MID$(GS$,8,1)="1"THEN OB=OB+1 
780 RETURN 

790 REM RESET SOUND CHANNELS 
800 OUTPN,159:OUTPN,191:OUTPN,223
810 OUTPN,255:RETURN

820 REM PLAY MUSIC 
830 CLS:PRINT"Playing music.":PRINT"------------"
840 IF FLAG=0 THEN PRINT:PRINT"Music array is empty.":GOSUB 1140:GOTO 30
850 FOR ZB=l TO 255
860 N1=X1(ZB):N2=X2(ZB):N3=X3(ZB):ZC=W(ZB):IF Nl=0 AND N2=0 AND N3=0 THEN ZB=255:GOTO 880
870 GOSUB 390:S0UND 0
880 NEXT ZB
890 GOTO 30
900 REM Create music
910 CLS:PRINT"Create Music.":PRINT"-------------":PRINT:GOSUB 1140
920 INPUT"How many notes to play.";ZA
930 IF ZA>255 THEN GOTO 920
940 FOR ZB=1 TO ZA
950 GOSUB 200
960 X1(ZB)=N1:X2(ZB)=N2:X3(ZB)=N3:W(ZB)=ZC:TZ(ZB)=FT
970 NEXT:X1(ZB)=0:X2(ZB)=0:X3(ZB)=0
980 GOSUB 1140:FLAG=1:GOTO 30
990 STOP

1000 REM Edit music
1010 CLS:PRINT "Edit Music.":PRINT"-----------":PRINT:IF FLAG=0 THEN PRINT"Buffer Is empty."GOSUB 1140:GOTO 30
1020 PRINT "Freq bytes can only be changed, not"
1030 PRINT "inserted. Use the";CHR$(&H8E);" key to change a ":PRINT "tone, else ";CHR$(&H8F);" key to move to the next":PRINT "tone, and CR to abort."
1040 FOR ZB=1 TO 255
1050 PRINT "Tone ";ZB;" is ";TZ(ZB);"Hz"
1055 PRINT "Wait period is";W(ZB)
1060 TR$="":TR$=INKEY$
1090 IF TR$=CHR$(30) THEN GOSUB 1150:GOTO 1050
1100 IF TR$=CHR$(29) THEN GOSUB 1140:NEXT
1110 IF TR$=CHR$(13) THEN 1130
1120 GOTO 1060 
1130 GOSUB 1140:GOTO 30 
1140 FOR DE=1 TO 200:NEXT DE:RETURN
1150 GOSUB 1140 :GOSUB 200 :X1(ZB)=Nl:X2(ZB)=N2:X3(ZB)=N3:W(ZB)=ZC:TZ(ZB)=FT:RETURN

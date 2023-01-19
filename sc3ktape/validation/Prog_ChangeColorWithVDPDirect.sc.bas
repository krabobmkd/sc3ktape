10 SCREEN 1,1:CLS 
20 PRINT "This is actually black writing" 
30 PRINT "On a green background."
40 FOR X = &HA000 TO &HA00C 
50 READ AA : POKE X,AA : NEXT X 
60 FOR DE = 1 TO 500 : NEXT DE 
70 CALL &HA000 : PRINT "But is it rea11y?"
80 GOTO 80 
90 DATA 243,219,191,62,33,211,191 
100 DATA 62,135,211,191,251,201 
110 REM Disable interrupts, read status register 
120 REM LD A with green/black(&H21), Out(&HBF) A 
130 REM LD A with register destination
140 REM Out(&H8F) A, Enable int/s, Return

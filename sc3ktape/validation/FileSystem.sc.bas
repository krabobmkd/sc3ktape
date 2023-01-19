	gosubfunc1
label10:
	screen1,1:cls
	print"           FILING CABINET":print
	print"      ### MENU ###":print
	print"1 READ A FILE"
	print"2 WRITE A FILE"
	print"3 ADJUST A FILE"
	print"4 VEIW THE FILE NAMES"
	print"5 SAVE THE FILES ON TAPE"
	print"6 DELETE A FILE"
	print"7 SEARCH FILES"
	print

	input"TYPE THE NUMBER OF YOUR CHOICE ";A
	onAgoto label2,label3,label4,label5,label6,label7,label8,label9
label9:
	print"ILLEGAL CHOICE TRY AGAIN"
	forDE=1to250:nextDE:gotolabel10
func1:
	erase:restore:ldata11readN
	dimD(N+1),A$(N+1),B$(N+1,3),F(N+1),F$(N+1)
	forT=1toN
	readD(T),A$(T),B$(T,1),B$(T,2),B$(T,3)
	next
	return

label2:
	gosubfunc12
	cls
	printA$(T)
	printB$(T,1);
	printB$(T,2);
	printB$(T,3);
	cursor0,21
	print"HIT SPACE TO VIEW ANOTHER FILE OR M TO VIEW THE MENU OR P FOR A PRINTOUT"
	forQ=1to17:cursor37,3+Q:printchr$(13)+chr$(10):next
	IN$=inkey$
	ifIN$=" "then2000
	ifIN$="M"then20
	ifIN$<>"P"then2050
	lprint"FILE NAME ";A$(T)
	lprint
	lprintB$(T,1)
	lprintB$(T,2)
	lprintB$(T,3)
	lprint
	gotolabel10
label3:
	cls
	print"CREATE A FILE"
	N=N+1:D(N)=D(N-1)+1:T=N
	input"NAME OF FILE ";A$(N)
	print"TYPE IN THE DATA"
label13:
	X=0:LE=0:Z=1:cursor0,4
	SP$="                                      "
	B$(T,1)="":B$(T,2)="":B$(T,3)=""
	X=X+int(len(IN$)/38)
	input"";IN$
	ifIN$=""then3180
	R$=right$(SP$,38-len(IN$)38)
	IN$=IN$+R$
	LE=len(B$(T,Z))+len(IN$)
	ifLE<255then3140
	Z=Z+1

	ifZ>3then3160
	B$(T,Z)=B$(T,Z)+IN$
	ifX<17then3070
	cursor0,22
	print"FILE FULL"
	cursor16,22
	input"ACCEPT DATA ? Y/N ";IN$
	ifIN$="Y" then 4100
	cursor0,3
	goto label13
label4:
	cls
	gosubfunc12
	cls:print"CORRECT THE DATA BY TYPING OVER IT"
	print:print:print
	forZ=1to3
	forP=0tolen(B$(T,Z))step38
	printmid$(B$(T,Z),P+1,37)+chr$(13)
	nextP,Z
	gotolabel13
	cls
	cursor0,1
	print20000+D(T)*10;"DATA";D(T);chr$(44);chr$(34);A$(T);chr$(34)
	print20001+D(T)*10;"DATA";chr$(34);B$(T,1);chr$(34)
	print"20000 DATA";N
	print"GOTO 4210"
	cursor10,22
	print"HIT CR UNTIL MENU APPEARS"
	cursor0,0
	end
	cls
	cursor0,1
	print20002+D(T)*10;"DATA";chr$(34);B$(T,2);chr$(34)
	print20003+D(T)*10;"DATA";chr$(34);B$(T,3);chr$(34)
	print"RUN"
	cursor10,22
	print"HIT CR UNTIL MENU APPEARS"
	cursor0,0
	end
label6:
	cls:cursor0,5
	print"THIS ROUTINE SAVES THE FILES AND"
	print"PROGRAMS ON TAPE.REWIND THE TAPE TO"
	print"THE START OF THE PROGRAM,CHECK THAT"
	print"THERE IS A LEAD IN THE 'OUT' PORT "
	print"ON THE COMPUTER AND IN 'MIKE' ON "
	print"THE TAPE RECORDER"
	print"THEN PRESS PLAY/RECORD"
	print"IF THIS IS ALL COMPLETE THEN PRESS CR"
	print:print
	print"SAVE";chr$(34);"FILE";chr$(34)
	cursor0,14
	end
label5:
	cls
	print" FILE NAMES"
	print

	forT=1toN
	ifinkey$=" "then6045
	printA$(T),
	next
	cursor0,21
	print"HIT P FOR A PRINTOUT OR M TO RETURN TO THE MENU"
label14:
	IN$=inkey$
	ifIN$="P"then 6200
	ifIN$="M"then 20
	gotolabel14
	lprint" FILE NAMES"
	lprint
	forT=1toN
	lprintA$(T),
	next
	lprint
	gotolabel10
label7:
	cls
	print"FILE ERASE"
	gosubfunc12
	cursor0,5
	print20000+D(T)*10
	print20001+D(T)*10
	print20002+D(T)*10
	print20003+D(T)*10
	print"20000 DATA";N-1
	print"RUN "
	cursor10,22
	print"HIT CR UNTIL MENU APPEARS";
	cursor0,4
	end
label8:
	cls
	print" FILE SEARCH "
	print"TYPE IN THE WORD YOU WANT TO SEARCH "
	input"FOR ; ";IN$
	print"ALL THE FOLLOWING FILES MAKE"
	print"REFERENCE TO ";chr$(39);IN$;" ";chr$(39)
	Q=0
	forT=1toN
	forZ=1to3
	F1=len(IN$)
	FF=len(B$(T,Z))-FI
	forP=1toFF
	ifIN$<>mid$(B$(T,Z),P,F1)then8130
	Q=Q+1:F(Q)=T:F$(Q)=A$(T)
	printA$(T),:gotolabel15
	nextP,Z
label15:
	nextT

	cursor0,21
	print"HIT P FOR A PRINT OUT OR M TO RETURN  TO THE MENU"
label16:
	II$=inkey$
	ifII$=""then8165
	ifII$="P"then8200
	ifII$="M"then20
	gotolabel16
	lprint"FILES WITH REFERENCE TO ";IN$:lprint
	ifQ=0then8250
	forT=1toQ
	lprintF$(T),
	next
	lprint
	gotolabel10
	lprint"NO REFERENCES FOUND"
	lprint
	gotolabel10
func12:
	input"TYPE IN THE FILE NAME ";IN$
	forT=1toN
	ifIN$<>left$(A$(T),len(IN$))then 10060
	printA$(T)
	input"ACCEPT OR REJECT A/R ";II$
	ifII$="R"then10060
	return

	next
	print"FILE NOT FOUND"
	for T=1to300:next
	gotolabel10
ldata11:
	data 2
	data 1,"PROGRAM"
	data"************************************  *                                  *  *       This is a test file.       *  *                                  *  *          DO NOT DELETE!          *  *                                  *  "
	data"*  Deleting this file will cause   *  *     the SEGA FILE SYSTEM to      *  *        require reloading.        *  *                                  *  ************************************  "
	data""
	data 5,"SUBARU"
	data"PURCHASED 28/11/88 1600 LEONE 1984    COST $12000 ($7000 TRADE-IN $5000     CASH) ENGINE No586223 REG.LS5880      CHASSIS NAB2E-12278. THIRD OWNER.     KEY No. W903.MILAGE 57191 KILOMETRES  SUNROOF 9/12/88 $320. WIRED FOR VAN   "
	data"HIGH BRAKE LIGHT FITTED 10/12/88 $40  "
	data""

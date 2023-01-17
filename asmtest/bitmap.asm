
;.define SegaBasicBase $9800

; JR Relative jump
; JR takes up one less byte than JP, but is also slower. Weigh the needs of the code at the time
; before choosing one over the other (speed vs. size).

; JP Absolute jumps to the address. Can be conditional or unconditional.
; JP takes one more byte than JR, but is also slightly faster,
; JP (HL), JP (IX), and JP (IY) are unconditional and are the fastest jumps, and do not take more bytes than other jumps.

; set n,reg bit a 1
;res (reset) bit a zero
; bit (btst , flag z) 

.memorymap
defaultslot 0
slotsize $ffff
slot 0 $0000
;slot 1 $4000
;slot 2 $8000
.endme


.rombankmap
bankstotal 1
banksize $ffff
banks 1
.endro

.define VDPControl $bf
.define VDPData $be
.define VDPScanline $7e
.define PSGPort $7f
.define IOPortA $DC
.define VRAMRead $0000
.define VRAMWrite $4000
.define CRAMWrite $c000


.macro SetVDPAddress args addr  ; c36
; sets the VDP address
    ld a, <addr
    out (VDPControl),a
    ld a, >addr
    out (VDPControl),a
.endm

.macro CopyToVDP
; copies data to the VDP
; parameters: hl = data address, de = data length
; affects: a, hl, bc, de
    ld c, VDPData
-:  outi ; Reads from (HL) and writes to the (C) port. HL is then incremented, and B is decremented.
    dec de
    ld a,d
    or e
    jp nz,-
.endm

;.bank 1 slot 1
	.org $981B
testd:
main:
	di

    ; set up VDP registers

    ld hl,VDPInitData
    ld b,VDPInitDataEnd-VDPInitData
    ld c,VDPControl
    otir	
 
    ; load tiles (Background)
;    SetVDPAddress $0000 | VRAMWrite
;    ld hl,Bitmap
;    ld de,BitmapEnd-Bitmap
;    CopyToVDP
    SetVDPAddress $0000 | VRAMWrite
	ld hl,Bitmap
	ld b,BitmapEnd-Bitmap
	ld c,VDPData
	otir	; Reads from (HL) and writes to the (C) port. HL is incremented and B is decremented. Repeats until B = 0.

;otdr ; Reads from (HL) and writes to the (C) port. HL and B are then decremented. Repeats until B = 0.

    ; out (imm8),a ; Writes the value of the second operand into the port given by the first operand.
    ; out (c),reg8


    ; load tilemap (Background)
 ;   SetVDPAddress $3800 | VRAMWrite
 ;   ld hl,testdata
 ;   ld de,dataend-testdata
 ;   CopyToVDP


	ei
;	jp justafter ; jump adress are 2b absolute according to org.
;	nop
;justafter:
;	call other ; calls adress are 2b absoluete according to org.
	ret

	;bc
	;de
	;hl
	;a
	; ''indexé
	; ix -> can be used for stack
	; iy -> can be used for stack
	; sp
	; i 'interupt vector'

;ADD A,im8
;ADD A,reg8
;ADD A,(reg16)
;ADD HL,HL ou BC ou DE ou SP
;ADD IX,BC ou DE ou IX ou SP
;ADD IY,BC ou DE ou IY ou SP
;SUB 5    ; équivalent à SUB A,5
;SUB D    ; équivalent à SUB A,D
;SUB (HL) ; équivalent à SUB A,(HL)

; only can
;add a,a
;add a,b
;add a,c
;add a,d
;add a,e
;add a,h
;add a,l
;add a,ixh
;add a,ixl
;add a,iyh
;add a,ixl
;add a,(hl)
;add a,(ix+n)
;add a,(iy+n)
;add a,n        ;8-bit constant

;add hl,bc
;add hl,de
;add hl,hl
;add hl,sp

;add ix,bc
;add ix,de
;add ix,ix
;add ix,sp

;add iy,bc
;add iy,de
;add iy,iy
;add iy,sp

.define dc_dataend $0
.define dc_ $2
.define dc_structsize $4

.define tempmem $0000

decomp:
	; stack thing
;	ld	ix,0
;	add	ix,sp
;	ld	iy, -dc_structsize
;	add	iy, sp
;	ld	sp, iy

	; params:
	; hl comp data start
	; c index of id to decomp.

	; todo loop for hl to reach c, then c free

	; hl comp data
	; -- find end of comp data
	ld c,(hl) ; low
	inc hl
	ld b,(hl) ; hi
	inc hl

	push hl
	add hl,bc
	ld (tempmem+dc_dataend),hl
	pop hl

	; - - - set VDP write adress
	di
		; The lower eight bits are written first.
		ld c,VDPControl
		ld b,(hl) ; low
		inc hl
		out (c),b
		ld b,(hl) ; hi
		inc hl
		out (c),b
    ei



    ; a: multi purpose and/or/add
    ;b
    ;c
    ;d - nbpart
    ;e - flags

    ld d,1 ; means: get new flags now

decomploop:
    ; compare ptr trick
    ld bc,(tempmem+dc_dataend)
    or a ; clear carry flag
    sbc hl,bc
    add hl,bc
    ; test end
    jp NZ,decomploop_end ; NZ C

    ; - - -  in all cases, flags shifts 2 bits
    srl e
    srl e
    ; - - - test if need new flag

    dec d
    jp NZ,nonewflags

    ld d,4+1
    ld e,(hl)
    inc hl

    ;ld a,e
    ;ld e,a
nonewflags:

    ;#define CD_MEMSET 0
    ;#define CD_COPY 1
    ; 4 cases -- test flag
    bit 0,e
    jp Z,dcmp_memset

dcmp_copy:
	; - - -  - - copy
	; note: otir will write "b" bytes to video adress set at begining of decomp.
	ld b,(hl)
	inc hl
	ld c,VDPData
	; may need + 1 byte copied here
	otir	; Reads from (HL) and writes to the (C) port. HL is incremented and B is decremented. Repeats until B = 0.

    jp  decomploop
dcmp_memset:
	;  - - - - - - memset
	ld b,(hl) ; length
	inc hl
	ld a,(hl) ; value
	inc hl
	ld c,VDPData
dcmp_msetloop:
	out (c),a
	dec b
	jp NZ,dcmp_msetloop

    jp  decomploop
decomploop_end:

; stack thing
;	ld	iy, dc_structsize
;	add	iy, sp
;	ld	sp, iy
	ret
setDefaultTileName:
	; TODO: easy !

	ret


; - - - - -
;reg0: 2:mode2
;reg1
; 1110 0000 <-graphics (16k,enabledislay,enableframeinterupt,9918mode1)
;						(mode3,noeffect,large16x16sprite,stretchd sprites )
; 1110 0010
; reg2 name table base adress 4b
; reg2 sur elevato action switch 0E/0F pour du double buff ?

;reg7 overscan & backdrop color: normal si tjrs different.

;meka for screen2: 02 E0 0E FF 03 76 03 05   00 00 FF
; meka bank panic: 02 E2 0E FF 03 76 03 00   00 00 FF
;                                       F0
;                           FF 03 7F   <-exerion chaneg reg5(sprite attribs base adr)
;elevatoraction: 02 E2 screen title, 02 82 when screen switch (switch off display & screen interupt)
VDPInitData:
.db $02,$80
.db $E2,$81
.db $56,$87
;.db $14,$80,$00,$81,$ff,$82,$ff,$85,$ff,$86,$ff,$87,$00,$88,$00,$89,$ff,$8a
VDPInitDataEnd:
Bitmap:
.db $98,$76,$54
BitmapEnd:

testdata:
	.db "CAN WO"
dataend:
	.db $10
	.db $11
;re .include graphics.asm

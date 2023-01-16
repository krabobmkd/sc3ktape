
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
	otir

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
;other:
;	ret
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
.include graphics.asm

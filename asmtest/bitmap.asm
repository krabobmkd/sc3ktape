
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

.include basicinit.basicoffsets.i


; - - BasicLevel3 values
.define blv3_u8_framecount $8b33

.define blv3_f_wait_bc $3a03


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


.define joy_btA 0

; a get: up1 down1 left1 right1 fl1 fr1 | up2 down2
.macro CheckJoystick1
    ld a,$92    ; 1001 0010 (write|b4:CtrlA|b3:CtrlC up|b1: CtrlB | b0: trlC low)
    out ($df),a ; pia_control register
    ld a,$07 ; Y line
    out ($de),a ; pia_portC_output_y_kb
    in a,($dc) ; pia_portA_input_x
.endm

.macro CheckKb_space
    ld a,$92    ; 1001 0010 (write|b4:CtrlA|b3:CtrlC up|b1: CtrlB | b0: trlC low)
    out ($df),a ; pia_control register
    ld a,$01 ; Y line
    out ($de),a ; pia_portC_output_y_kb
    in a,($dc) ; pia_portA_input_x

    ;cpl a
    bit 4,a ; space

.endm

.macro CheckKb_break
    ld a,$92    ; 1000 0010 (write|b4:CtrlA|b3:CtrlC up|b1: CtrlB | b0: trlC low)
    out ($df),a ; pia_control register
    ld a,$06 ; Y line
    out ($de),a ; pia_portC_output_y_kb
    in a,($dd) ; pia_portB_input_x
    bit 0,a ; space
.endm

;first byte:
.define sn7_ldt $80
.define sn7_l_Chan0 %00000000
.define sn7_l_Chan1 %00100000
.define sn7_l_Chan2 %01000000
.define sn7_l_Chan3 %01100000
.define sn7_l_tVol  %00010000 ; will latch volume, else tone/noise
.define sn7_l_lodatamask %00001111
; second byte:
.define sn7_d_hidatamask %00111111

; If the currently latched register is a tone register then the low 6 bits of the byte (DDDDDD) are placed into the high 6 bits of the latched register.
; tones are 6+4=10 bits


	.org blv3start
main:

	call vdp_setBlank

    ;ld hl,bmcdata ;skull_bm_cdata
    ld hl,logo_bm_cdata
    call decomp_to_dvp

   ;ld hl,clcdata ;
	ld hl,logo_cl_cdata
    call decomp_to_dvp

	call vdp_set_screen2


mainloop:
;    ; wait til blank, Basic lv3 way
	ld hl,blv3_u8_framecount
	ld e,(hl)
-:
	ld a,(hl)
	cp e
	jp z,-


	CheckKb_space
	jp z,mainend
	CheckKb_break
	jp z,mainend

    jp mainloop
mainend:


	ret
vdp_setBlank:
	di
		ld hl,VDPRegs_blank
		ld b,VDPRegs_blank_end-VDPRegs_blank
		ld c,VDPControl
		otir
	ei
	ret
vdp_set_screen2:
	di
		ld hl,VDPRegs_screen2
		ld b,VDPRegs_screen2_end-VDPRegs_screen2
		ld c,VDPControl
		otir
	ei
	ret

decomp_to_dvp:
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
	ld (dc_writeendofdatahere+1),hl
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


    ; a: multi purpose and/or/add
    ;b
    ;c
    ;d - nbpart
    ;e - flags


    ld d,1 ; means: get new flags now
;breakpoint 9843
decomploop:
    ; compare ptr trick
dc_writeendofdatahere:
    ld bc,0
    or a ; clear carry flag
    sbc hl,bc
    add hl,bc
    ; test end
    ;jp NZ,decomploop_end ; NZ C
    jp Z,decomploop_end ; Z set if hl==bc

    ; - - -  in all cases, flags shifts 2 bits
    srl e
    srl e
    ; - - - test if need new flag

    dec d
    jp NZ,nonewflags

    ld d,4
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
	; - - -  - - copy - mean length is 1, special case
	; note: otir will write "b" bytes to video adress set at begining of decomp.
;	ld c,VDPData
	ld b,(hl)
	inc hl
	; a bit ugly test
	inc b
	dec b ; test 0b xb
	jp Z,dcmp_cp_l_is0
	; may need + 1 byte copied here
	ld c,VDPData
	otir	; Reads from (HL) and writes to the (C) port. HL i98s incremented and B is decremented. Repeats until B = 0.
dcmp_cp_l_is0:
	ld c,VDPData
	outi		; one more because b means [1,256]
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
	;dec b
	;jp NZ,dcmp_msetloop
	djnz dcmp_msetloop
	out (c),a  ; one more because b means [1,256]

    jp  decomploop
decomploop_end:
    ei
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
VDPRegs_screen2:
	.db $02,$80
	.db $E2,$81  ; 1110 0010     16k,enable display,enable vert. initerupt, large sprites
	.db $11,$87 ; bg colors
;.db $14,$80,$00,$81,$ff,$82,$ff,$85,$ff,$86,$ff,$87,$00,$88,$00,$89,$ff,$8a
VDPRegs_screen2_end:
VDPRegs_blank:
	.db $00,$80
	.db $80,$81 ; display off
VDPRegs_blank_end:

	.include krb_mkd5.sc2.asm
	;.include krb_cyberskull.sc2.asm
	;.include HABR0CON_by_Tutty.sc2.asm
	;.include mire.sc2.asm
; in basic tape mode, remaining ram data start is obviously here: NO, its var base ! -> get 3 bytes further.
;tempdata:

;testdata:
;	.db "CAN WO"
;dataend:
;	.db $10
;	.db $11
;re .include graphics.asm

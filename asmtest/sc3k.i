.ifndef _sc3k_i_
.define _sc3k_i_ 1

; - - - - BasicLevel3 values
.define blv3_u8_framecount $8b33

; - - - basic LV3 rom functions
.define blv3_f_wait_bc $3a03

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

; a get: up1 down1 left1 right1 fl1 fr1 | up2 down2
.macro CheckJoystick1
    ld a,$92    ; 1001 0010 (write|b4:CtrlA|b3:CtrlC up|b1: CtrlB | b0: trlC low)
    out ($df),a ; pia_control register
    ld a,$07 ; Y line
    out ($de),a ; pia_portC_output_y_kb
    in a,($dc) ; pia_portA_input_x
.endm

.define kb_portB $dd
.define kb_portC $de

.macro CheckKb args YLine portBorC XLineBit
    ld a,$92    ; 1001 0010 (write|b4:CtrlA|b3:CtrlC up|b1: CtrlB | b0: trlC low)
    out ($df),a ; pia_control register
    ld a,YLine ; Y line
    out (portBorC),a ; pia_portC_output_y_kb
    in a,($dc) ; pia_portA_input_x

    bit XLineBit,a ; space

.endm

.macro CheckKb_space
	CheckKb 1 kb_portC 4

;    ld a,$92    ; 1001 0010 (write|b4:CtrlA|b3:CtrlC up|b1: CtrlB | b0: trlC low)
;    out ($df),a ; pia_control register
;    ld a,$01 ; Y line
;    out ($de),a ; pia_portC_output_y_kb
;    in a,($dc) ; pia_portA_input_x

;    bit 4,a ; space

.endm

.macro CheckKb_break
	CheckKb 6 kb_portB 0

;    ld a,$92    ; 1000 0010 (write|b4:CtrlA|b3:CtrlC up|b1: CtrlB | b0: trlC low)
;    out ($df),a ; pia_control register
;    ld a,$06 ; Y line
;    out ($de),a ; pia_portC_output_y_kb
;    in a,($dd) ; pia_portB_input_x
;    bit 0,a ; space
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

.macro waitVBlank
    ; wait til blank, Basic lv3 way
	ld hl,blv3_u8_framecount
	ld e,(hl)
-:
	ld a,(hl)
	cp e
	jp z,-
.endm

.endif


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

VDPData: equ $be

CopyToVDP: macro
	ld c, VDPData
-:
	outi
	dec de
	ld a,d
	or e
	jp nz,-
	endm


;.macro CopyToVDP
; copies data to the VDP
; parameters: hl = data address, de = data length
; affects: a, hl, bc, de
;    ld c, VDPData
;-:  outi
;    dec de
;    ld a,d
;    or e
;    jp nz,-
;.endm

	org $980F
testd:
main:
	di

	CopyToVDP
	ei
	jp justafter ; jump adress are 2b absolute according to org.
	nop
justafter:
	call other ; calls adress are 2b absoluete according to org.
	ret
other:
	ret
; - - - - -
testdata:
	db "CAN WE"
dataend:
	db 0
	db $13
	db $37
	db 0

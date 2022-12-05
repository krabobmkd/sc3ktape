
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
	.org $980F
testd:
main:
	di

    ; load tiles (Background)
;    SetVDPAddress $0000 | VRAMWrite
;    ld hl, FiftyTiles
;    ld de, FiftyTilesSize
;    CopyToVDP

    ; load tilemap (Background)
    SetVDPAddress $3800 | VRAMWrite
    ld hl,testdata
    ld de,dataend-testdata
    CopyToVDP


	ei
;	jp justafter ; jump adress are 2b absolute according to org.
;	nop
;justafter:
;	call other ; calls adress are 2b absoluete according to org.
	ret
;other:
;	ret
; - - - - -
testdata:
	.db "CAN WE"
dataend:
	.db $10
	.db $11

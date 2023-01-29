; - - -source for demo screen with gfx and sound and keyb/joy inputs

decomp_to_dvp:
	; todo loop for hl to reach c, then c free

	; hl comp data
	; -- find end of comp data
	ld c,(hl) ; low
	inc hl
	ld b,(hl) ; hi
	inc hl

	push hl
	add hl,bc
	.ifdef AUTOMODIF
		ld (dc_writeendofdatahere+1),hl
	.else
		ld (dd_wtempx),hl
	.endif
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

	.ifdef AUTOMODIF
dc_writeendofdatahere:
    ld bc,0
	.else
	ld bc,(dd_wtempx)
	.endif


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
	otir	; Reads from (HL) and writes to the (C) port. HL is incremented and B is decremented. Repeats until B = 0.
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

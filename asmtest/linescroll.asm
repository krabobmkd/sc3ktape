
scroll_line_ram:
	;hl line start in ram
.define w_linebufferptr dd_wtempx
.define w_charbufferptr dd_wtempy

	; point last of first line, because write decreasing to scroll left.
	ld hl,dd_linebuffer+32-1
	ld (w_linebufferptr),hl

	ld hl,dd_newchar
	ld (w_charbufferptr),hl

	; update new char if needed.
	ld a,(dd_newcharcount)
	inc a
	cp a,8
	jp nz,nonewchar
		; - - read next char
		ld hl,(dd_textptr)
		ld a,(hl)
		inc hl
		or a ; tests
		jp nz,charok
			; 0 mean text restart
			ld hl,(dd_textptrstart)
			ld a,(hl)
charok:
		ld (dd_textptr),hl
		; hl free
		; here a is ascii
		sub a,32 ; first char to zero [32,96] -> [0,63]
		ld h,0
		ld l,a
		add hl,hl ; *8 char size ->[0->512-8]
		add hl,hl
		add hl,hl
		ld bc,ext_charset
		add hl,bc ; hl points char
		; LD (DE),(HL),
		ld de,dd_newchar
		ld bc,8 ; or 7 ?
		ldir

		xor a ; clear a, count 8 pixels stil next
		;inc hl
nonewchar:
	ld (dd_newcharcount),a

;ld bc,dd_newchar
	ld e,7
slr_yloop:

	; carry is new bit at right.
	ld hl,(w_charbufferptr)

;gothere: ; to get debug symbol
	ld a,(hl)
	rlc a
	ld (hl),a
;	rlc (hl) ; rotate left and carry

	inc hl
	ld (w_charbufferptr),hl

	; hl is decr. line ptr
	ld hl,(w_linebufferptr)
	; xloop
	ld d,32/4
-:
		.repeat	4
		ld a,(hl)
		adc a,a
		ld (hl),a
		dec hl ; all flags preserved
		.endr
		dec d ; c flag preserved

	jp NZ,-
	; next line
	;ld ix,32
	;add hl,ix
	ld bc,32+32
	add hl,bc
	ld (w_linebufferptr),hl

	dec e
	jp NZ,slr_yloop

	ret

vdpcopy_color_line:
	SetVDPAddress ($2000+((8*32)*16)-1)

	; de free
	ld c,VDPData
	ld a,32
-:
	ld hl,colorchar
	ld b,8
	otir	; Reads from (HL) and writes to the (C) port. HL is incremented and B is decremented. Repeats until B = 0.
	dec a
	jp NZ,-

	ret
colorchar: .db $e0,$f0,$f0,$f0,$e0,$e0,$e0,$e0
; write 32*8 on linear VDP
; hl data to read
vdpcopy_from_linear_line:

	SetVDPAddress ($0000+((8*32)*16)-1)

	ld c,VDPData ; used by out as port id.

	;read ptr
	ld hl,dd_linebuffer
	;
	ld a,32 ; nb char on line
	; horizontal char loop
-:
	ld de,32-1 ; -1 because of outi
	ld b,8
	.repeat 7
	outi  ; aka ld a,(hl);out (c) a;inc  hl ...
	add hl,de
	.endr
	outi
	; resetb to line
	ld de,32-1-(32*8) +1 ; -1 because of out1
	add hl,de
	; horizontal char loop
	dec a
	jp NZ,-

	ret

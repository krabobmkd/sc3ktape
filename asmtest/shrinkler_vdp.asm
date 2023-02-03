; Shrinkler z80 decompressor Madram/OVL version v8
; Modified by Ivan Gorodetsky (2019-10-06)

; 212 bytes - z80 version

; port krb



.include sc3k.i

; you may change probs to point to any 256-byte aligned free buffer (size 2.5 Kilobytes)
; $1000:4k
.define probs $8000 ; $f600 ; max ram-2.kb $ $400=1k $800=2k $A00=2.5k $c00 = 3k
.define probs_ref probs+$400
.define probs_length probs_ref
.define probs_offset probs_length+$200

; all level3 takes 6ko
;level3A:7680b left if probs is $b600.
; basic level3A:
; $8000-$BFFF : RAM (16K)
;iternal ram level2:  $C000-$FFFF
; input IX=source
; input DE=destination		- it has to be even!
; call shrinkler_decrunch_ram
shrinkler_decrunch_ram:
	call shrinkler_decrunch_start

literal
		scf
getlit
		call nc,getbit
		rl l
		jr nc,getlit
		ld a,l
		exx
		ld (de),a
		inc de
		call getkind
		jr nc,literal
		ld h,probs_ref/256
		call getbit
		jr nc,readoffset
readlength
		ld h,probs_length/256
		call getnumber
		push hl
		add hl,de
		ldir
		pop hl
		call getkind
		jr nc,literal
readoffset
		ld h,probs_offset/256
		call getnumber
		ld hl,2
		sbc hl,bc
		exx
		jr nz,readlength

	ret

shrinkler_decrunch_vdp:
	call shrinkler_decrunch_start


literal_vdp
		scf
getlit_vdp
		call nc,getbit
		rl l
		jr nc,getlit_vdp
		ld a,l
		exx
;		ld (de),a
;		inc de
		out (VDPData),a

		call getkind
		jr nc,literal_vdp
		ld h,probs_ref/256
		call getbit
		jr nc,readoffset_vdp
readlength_vdp
		ld h,probs_length/256
		call getnumber
		push hl
		add hl,de
;		ldir ; LD (DE),(HL), then increments DE, HL, and decrements BC)


;	push c
;		ld c,VDPData

;		otir ;Reads from (HL) and writes to the (C) port. HL is incremented and B is decremented. Repeats until B = 0.
;   pop c
		pop hl
		call getkind
		jr nc,literal_vdp
readoffset_vdp
		ld h,probs_offset/256
		call getnumber
		ld hl,2
		sbc hl,bc
		exx
		jr nz,readlength_vdp
	ret
;  - - - - - - - in common
shrinkler_decrunch_start:
		exx ; exchanges BC, DE, and HL with shadow registers with BC', DE', and HL'.
		ld hl,10*256+probs
		ld bc,10*256+$80
		xor a
init
		dec h
iniloop
		ld (hl),a
		inc l
		jr nz,iniloop
		xor c
		djnz init
		ld d,b
		ld e,b
		push de
		pop iy			; d2=0
		inc e
		ld a,c
		ex af,af'
	ret
; - - - - - -

getnumber
_numberloop
		inc l
		inc l
		call getbit
		jr c,_numberloop
		exx
		ld bc,1
		exx
_bitsloop
		dec l
		call getbit
		exx
		rl c
		rl b
		exx
		dec l
		jr nz,_bitsloop
		exx
		ret
		
readbit
		ex de,hl
		add hl,hl
		ex de,hl
		ex af,af'
		add a,a
		jr nz,_rbok
		ld a,(ix)
		inc ix
		adc a,a
_rbok
		ex af,af'
		add iy,iy
		ex af,af'
		jr nc,$+4
		.db 0FDh,2Ch	; inc yl
		ex af,af'
		jr getbit

getkind
		ld a,e
		exx
		ld hl,probs
		rra
		jr nc,getbit
		inc h
		inc h
getbit
		ld a,d
		add a,a
		jr nc,readbit
		ld b,(hl)
		inc h
		ld c,(hl)
		push hl
		ld l,c
		ld h,b
		push bc
		ld a,%11100001
shift4
		srl b
		rr c
		add a,a
		jr c,shift4
		sbc hl,bc
		pop bc
		push hl
;--
		sbc hl,hl
muluw
		add hl,hl
		rl c
		rl b
		jr nc,_mulcont
		add hl,de
		jr nc,_mulcont
		inc bc
_mulcont
		dec a
		jr nz,muluw
		.db 0FDh,7Dh	; ld a,yl
		sub c
		ld l,a
		.db 0FDh,7Ch	; ld a,yh
		sbc a,b		
		jr nc,zero
one
		ld e,c
		ld d,b
;--
		pop bc
		ld a,b
		sub 0F0h
		ld b,a
		dec bc
		jr _probret

zero
		.db 0FDh,67h	; ld yh,a
		ld a,l
		.db 0FDh,6Fh	; ld yl,a

		ex de,hl
		sbc hl,bc

		ex de,hl
		pop bc
_probret
		pop hl
		ld (hl),c
		dec h
		ld (hl),b
		ret


		;.end

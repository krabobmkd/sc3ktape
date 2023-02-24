; included
; screen mode must not be blank so waitvblank works.
; blv3 version using famecounter from basic's vblank.
checkPalOrNtsc:
    ld hl,0
    ; wait til blank, Basic lv3 way
	ld bc,blv3_u8_framecount
	ld a,(bc)
	ld e,a
-:
	ld a,(bc)
	cp e
	jp z,-
	; here end of bitmap screen
	; --- then wait again next frame counting.
	ld e,a

-:
	inc hl		; 6
	ld a,(bc)	; 7
	cp e		; 4
	jp z,-		;10   ->27c

	; 3579545/60/27 ntsc -> 2209
	; 3546893/50/27 pal  -> 2627

	; compare bc
	;xor a
	ld (dd_isPal),0 ; default: ntsc

	ld de,2418 ; in between
	sbc hl,de
	ret c

	ld (dd_isPal),1	; pal

    ret

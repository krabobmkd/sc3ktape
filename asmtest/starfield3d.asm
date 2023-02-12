
; - - - - buffers
.define stf_base $8000
.define stf_projtable (stf_base+0)
; size=192*2
;NOT .define stf_y_bm_shift (stf_base+ $800)

; - - -
.define stf_nbStars 48

starfield3d_init:
	; double buffer for writting in bm...
	ld hl,dd_starbmA
	ld (dd_starbm1),hl
	ld hl,dd_starbmB
	ld (dd_starbm2),hl
	ld hl,dd_starbmC
	ld (dd_starbm3),hl
    ; - multiply proj table
    ld hl,stf_projtable ; write

    ld c,16 ; 16->1
stfi_loopy:
    ld de,projtab ; read ptr, 8b value for 16
stfi_loopx:
    push hl
	ld h,d
	ld l,e
    ld l,(hl)
    ; kindof 5b mul
       ; todo try to have a loop per bit.
    xor a

    bit 4,c
    jp z,no4
    add a,l
no4:
    srl l

    bit 3,c
    jp z,no3
    add a,l
no3:
    srl l

    bit 2,c
    jp z,no2
    add a,l
no2:
   srl l

    bit 1,c
    jp z,no1
    add a,l
no1:
   srl l

    bit 0,c
    jp z,no0
    add a,l
no0:
    pop hl
    ; write
    ld (hl),a
    inc hl
;  loop test
    inc de
    ld a,e
    cp <(projtab+128) ;just compare low byte+128, works because size <=256
    jp nz,stfi_loopx
; loop test
    dec c
    jp nz,stfi_loopy

; - - -  - - - -
	; set fixed x y coord for star as random, then write corresponding ptr in stf_projtable
	ld iy,rndt
	ld ix,dd_starpos
	ld a,0
	push af
-:

	; - - - - - - - - - Y
	pop af
	ld h,(iy+0)
	inc iy

	rl a ; rot previous
	add a,h

	push af
	and $0f
	ld d,a ; keep Y for things
	; a random with [0,15]

	; a*128
	ld h,a
	ld l,0
	srl h ; 16b shift right
	rr l	; hl= [0,15]*128

	ld bc,stf_projtable ; ad vec
	add hl,bc ; start ptr
	ld (ix+0),l
	ld (ix+1),h

	; - - - - - - - - - X
	pop af
	ld h,(iy+0)
	inc iy

	rl a ; rot previous
	add a,h

	push af
	and $0f
	; a random with [0,15]

	; rule: if Y too close to center, X must be wide, so less star "keep on center", better dispersion
	bit 3,d
	jp z,nocase
		res 3,a
nocase
	; a*128
	ld h,a
	ld l,0
	srl h ; 16b shift right
	rr l	; hl= [0,15]*128

	ld bc,stf_projtable ; ad vec
	add hl,bc ; start ptr
	ld (ix+2),l
	ld (ix+3),h

	; - - next
	ld bc,4 ; ad vec
	add ix,bc

	ld a,iyl
	cp <(rndt_end) ;just compare low byte+n*s, works because size <=256 or n
	jp nz,noresetrnd
		ld iy,rndt
noresetrnd
	; test loop
	ld a,ixl
	cp <(dd_starpos+(4*stf_nbStars)) ;just compare low byte+n*s, works because size <=256 or n
	jp nz,-

	pop af
    ret
rndt:
	.db $30,$e2,$57,$93,$01
rndt_end:
;; ===================================
    .struct starf
       ; space state
        ybstart dw
        xbstart dw
    .endst

starfield3d_frame_ram:


; e=z running
	ld hl,dd_mainz
	ld e,(hl) ; z 0->127, could use frame counter

    ld hl,dd_starpos

	ld ix,(dd_starbm1)

-
	res 7,e ; and $7f  , one time for y and x
	; - - - -  treat Y
	ld c,(hl)
	inc hl
	ld b,(hl)
	inc hl

	ld a,e	; add z to pointer
	add a,c
	ld c,a

	ld a,(bc) ; read projection table for y (0,127)
	; for Y only manage [0,95]
	cp 95
	jp nc,stf_invalid

	bit 2,l  ; 1/2 star got y inverted
	jp z,noneg_y
		neg
noneg_y
	add a,96 ; [0,191]   y coord of star

	; screen coord
	ld d,a
	srl d
	srl d
	srl d	; ___YYYYY XXXXXyyy
	; high ___YYYYY can be written here
	ld (ix+1),d

	and %00000111
	ld d,a ;  _____yyy

	; - - - -  treat x now
	ld c,(hl) ; get x on proj pointer
	inc hl
	ld b,(hl)
	inc hl

	ld a,e	; add z on base
	add a,c
	ld c,a
	ld a,(bc) ; read projection table for x (0,127)
	bit 3,l ; neg x sometimes (2/4) to have 4 quarters
	jp z,noneg_x
		neg
noneg_x
	; a [-128,127]
	add a,128 ;  [0,255] x screen coord
	ld c,a
	and %11111000

	or a,d ; XXXXXyyy
	; low XXXXXyyy can be written here
	ld (ix+0),a
	inc ix
	inc ix

	; todo bm value:
;	ld a,$14
;	ld (ix+0),a
;	inc ix
	; - -  sure this could be optimized
	ld a,c ; re-x
	and %00000111
	ld c,a ; low8 x
	ld b,0
	ld iy,shiftdotdb
	add iy,bc ; just need +0+7 , but 16 b because of adress.-> or use $8000 temp

	ld a,(iy+0)
	ld (ix+0),a ; write
	inc ix

	jp stf_ok
stf_invalid
	; so 0
	ld (ix+0),0 ; will write 0 at bm 0
	inc ix
	ld (ix+0),0
	inc ix
	ld (ix+0),0
	inc ix

	inc hl	; the unused x ptr
	inc hl
	;ld bc,_sizeof_starf-2 ; ad vec
	;add hl,bc
stf_ok
	; - - - - -- - -
	inc e ; z
	inc e
    ; - - next
	;ld bc,_sizeof_starf ; ad vec
	;add hl,bc
	; test loop
	ld a,l
	cp <(dd_starpos+(4*stf_nbStars)) ;just compare low byte+n*s, works because size <=256 or n
	jp nz,-

	; inc z for next time
	ld hl,dd_mainz
	inc (hl)

	; swap double buffer for writting in bm...
;	ld bc,(dd_starbm1)
;	ld de,(dd_starbm2)
;	ld (dd_starbm2),bc
;	ld (dd_starbm1),de

 ; starbm1 just written -> to bm3
 ;bm2 -> to bm1
 ;bm3 ->to bm2
	; roll triple buffer
	ld bc,(dd_starbm1)
	ld de,(dd_starbm2)
	ld hl,(dd_starbm3)

	ld (dd_starbm3),bc
	ld (dd_starbm1),de
	ld (dd_starbm2),hl

    ret
shiftdotdb:
	.db $80,$40,$20,$10,$08,$04,$02,$01
starfield3d_frame_vdp:

; = = = = = = = = = == = = erase previous points

	ld hl,(dd_starbm2)  ; & or 2 previous frame
	call starfield3d_frame_vdpB
	ld hl,(dd_starbm3)  ; current frame
	call starfield3d_frame_vdpB
	ret
starfield3d_frame_vdpB:
; - - -= = = = = = = = = = = = == == = =  write new points

	;ld ixl,stf_nbStars
	ld bc,(stf_nbStars<<8) | $bf ; VDPControl  ; VDPData be		; VRAMWrite
-
	;  - - - set read adress
	; "The lower eight bits are written first. "
	ld a,(hl)
	cp 0
	jp z,invaliddot
	inc hl
	ld d,(hl)

	out (c),a ; $bf VDPControl

	dec hl ; because reread for write
	out (c),d ; $bf VDPControl
	;read
	dec c
	in e,(c) ; e prev bm value
	inc c

	;  - - - set write adress
	; "The lower eight bits are written first. "
	ld a,(hl) ; written backward big endian
	inc hl
	out (c),a ; $bf VDPControl

	;opt?
	ld a,(hl)
	or $40 ; for VRAMWrite
	inc hl
	out (c),a ; $bf VDPControl

	; - -  - write data
	ld a,(hl) ; bm star value
	inc hl
	xor e ; xor original bm

	dec c	; magic, -> $be VDPData
	out (c),a
	inc c ; re,  -> $bf VDPControl

	djnz -
	jp afterdot

invaliddot
	inc hl
	inc hl
	inc hl
	djnz -
afterdot

	ret


; - - - -
projtab:
	.incbin res/projection128.cbin


; - - - - buffers
.define stf_base $8000
; 128*16
.define stf_projtable (stf_base+0)
; 128
.define stf_framedroptable (stf_base+(128*16))

; - - -
.define stf_nbStars 16 ; pal 52

.define stf_w_ybstart 0
.define stf_w_xbstart 2

.define stf_w_bmadrA  4
.define stf_b_bmvalA  6

.define stf_w_bmadrB  7                                                                                                   
.define stf_b_bmvalB  9
.define stf_b_bframedrop  10
.define stf_size 11


starfield3d_init:
	; double buffer for writting in bm...
;	ld hl,dd_starbmA
;	ld (dd_starbm1),hl
;	ld hl,dd_starbmB
;	ld (dd_starbm2),hl
;	ld hl,dd_starbmC
;	ld (dd_starbm3),hl
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

;---- init framedrop table
	; set 1 when no move until next value
	ld ix,stf_framedroptable
    ld hl,projtab ;
	ld b,127

	ld (ix+0),0
	inc ix

	ld c,(hl)
-:

	inc hl
	ld a,(hl)
	cp c
	jp z,cc2
		ld (ix+0),1
	jp cc1
cc2
	ld (ix+0),0
cc1

	ld c,a

	inc ix


	djnz -


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
	ld (ix+stf_w_ybstart),l
	ld (ix+stf_w_ybstart+1),h

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
	ld (ix+stf_w_xbstart),l
	ld (ix+stf_w_xbstart+1),h

	; - - next
	ld bc,stf_size ; ad vec
	add ix,bc

	ld a,iyl
	cp <(rndt_end) ;just compare low byte+n*s, works because size <=256 or n
	jp nz,noresetrnd
		ld iy,rndt
noresetrnd
	; test loop
	ld a,ixl
	cp <(dd_starpos+(stf_size*stf_nbStars)) ;just compare low byte+n*s, works because size <=256 or n
	jp nz,-

	pop af
    ret
rndt:
	.db $30,$e2,$57,$93,$01
rndt_end:
;; ===================================
;.define stf_nbStars 48

;.define stf_w_ybstart 0
;.define stf_w_xbstart 2

;.define stf_w_bmadrA  4
;.define stf_b_bmvalA  6

;.define stf_w_bmadrB  7
;.define stf_b_bmvalB  9
;.define stf_b_bframedrop  10
;.define stf_size 11

starfield3d_frame_ram:


; e=z running
	ld hl,blv3_u8_framecount
	ld e,(hl) ; z 0->127, could use frame counter

	ld l,0 ; just to count
	ld ix,dd_starpos
	; hl free
-
	res 7,e ; and $7f  , one time for y and x

	; test framedrop
	ld d,0 ;de !
	ld iy,stf_framedroptable ; is 256 aligned
	add iy,de
	ld a,(iy+0)
	ld (ix+stf_b_bframedrop),a
	bit 0,a
	jp z,stf_invalid

	; - - - -  treat Y
	ld c,(ix+stf_w_ybstart+0)
	ld b,(ix+stf_w_ybstart+1)

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
	ld (ix+stf_w_bmadrA+1),d

	and %00000111
	ld d,a ;  _____yyy

	; - - - -  treat x now
	ld c,(ix+stf_w_xbstart+0) ; get x on proj pointer
	ld b,(ix+stf_w_xbstart+1)

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
	ld (ix+stf_w_bmadrA+0),a

	; - - find x value to write in bm, sure this could be optimized
	ld a,c ; re-x
	and %00000111
	ld c,a ; low8 x
	ld b,0
	ld iy,shiftdotdb
	add iy,bc ; just need +0+7 , but 16 b because of adress.-> or use $8000 temp
	ld a,(iy+0)


	ld (ix+stf_b_bmvalA),a ; value to write

	jp stf_ok
stf_invalid
	ld (ix+stf_w_bmadrA+1),$80
	;ld (ix+stf_b_bmok),0 ; would mean not OK
stf_ok
	; - - - - -- - -
	inc e ; z
	inc e
	inc e
    ; - - next
	ld bc,stf_size ; ad vec
	add ix,bc
	inc l
	; test loop
	ld a,ixl
	cp <(dd_starpos+(stf_size*stf_nbStars)) ;just compare low byte+n*s, works because size <=256 or n
	jp nz,-

	; inc z for next time
	;ld hl,dd_mainz
	;inc (hl)

	; swap double buffer for writting in bm...
;	ld bc,(dd_starbm1)
;	ld de,(dd_starbm2)
;	ld (dd_starbm2),bc
;	ld (dd_starbm1),de

 ; starbm1 just written -> to bm3
 ;bm2 -> to bm1
 ;bm3 ->to bm2
	; roll triple buffer
;	ld bc,(dd_starbm1)
;	ld de,(dd_starbm2)
;	ld hl,(dd_starbm3)

;	ld (dd_starbm3),bc
;	ld (dd_starbm1),de
;	ld (dd_starbm2),hl

    ret
shiftdotdb:
	.db $80,$40,$20,$10,$08,$04,$02,$01
starfield3d_frame_vdp:

;.define stf_w_ybstart 0
;.define stf_w_xbstart 2

;.define stf_w_bmadrA  4
;.define stf_b_bmvalA  6

;.define stf_w_bmadrB  7
;.define stf_b_bmvalB  9
;.define stf_b_bframedrop  10
;.define stf_size 11

; - - -= = = = = = = = = = = = == == = =  write new points

	;ld ixl,stf_nbStars
	ld ix,dd_starpos
	ld bc,(stf_nbStars<<8) | $bf ; VDPControl  ; VDPData be		; VRAMWrite
-
	bit 7,(ix+stf_w_bmadrA+1)
	jp nz,nextdot
	;ld (ix+stf_b_bframedrop),a
	;bit 0,a
	;jp nz,nextdot

	; - - - - - - - remove previous
	; "The lower eight bits are written first. "
	ld d,(ix+stf_w_bmadrB+0)
	out (c),d ; $bf VDPControl
	ld a,(ix+stf_w_bmadrB+1)
	out (c),a ; $bf VDPControl
	;read
	dec c
	in e,(c) ; e prev bm value
	inc c

	;  - - - set write adress
	; "The lower eight bits are written first. "
	out (c),d ; $bf VDPControl
	or $40 ; for VRAMWrite
	out (c),a ; $bf VDPControl

	; - -  - write data
	ld a,(ix+stf_b_bmvalB) ; bm star value
	xor e ; xor original bm

	dec c	; magic, -> $be VDPData
	out (c),a
	inc c ; re,  -> $bf VDPControl

	; = = = = = = = = = = = = =
	;  - - - set read adress
	; "The lower eight bits are written first. "
	ld d,(ix+stf_w_bmadrA+0)
	ld (ix+stf_w_bmadrB+0),d

	out (c),d ; $bf VDPControl
	ld a,(ix+stf_w_bmadrA+1)
	ld (ix+stf_w_bmadrB+1),a
	out (c),a ; $bf VDPControl
	;read
	dec c
	in e,(c) ; e prev bm value
	inc c

	;  - - - set write adress
	; "The lower eight bits are written first. "	
	out (c),d ; $bf VDPControl
	or $40 ; for VRAMWrite
	out (c),a ; $bf VDPControl

	; - -  - write data
	ld a,(ix+stf_b_bmvalA) ; bm star value
	ld (ix+stf_b_bmvalB),a
	xor e ; xor original bm

	dec c	; magic, -> $be VDPData
	out (c),a
	inc c ; re,  -> $bf VDPControl
nodraw

nextdot
	ld de,stf_size ; ad vec
	add ix,de

	djnz -

	ret


; - - - -
projtab:
	.incbin res/projection128.cbin

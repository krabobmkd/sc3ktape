
; - - - - buffers
.define stf_base $8000
.define stf_projtable (stf_base+0)
.define stf_y_bm_shift (stf_base+ $800)

; - - -
.define stf_nbStars 16

starfield3d_init:

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
; - - -loop test
    inc de
    ld a,e
    cp >(projtab+128) ; works because size <=256
    jp nz,stfi_loopx
; - - loop test
    dec c
    jp nz,stfi_loopy
; - - - - - - -- 128*16 written = 2kb
    ; then table stf_y_bm_shift for 0->191
    ; looks:   ___YYYYY _____yyy
    ;ld bc,0
    ld c,0
    ;xor a
-:
    ld b,c
    srl b
    srl b
    srl b ; ___YYYYY
    ld (hl),b
    inc hl

    ld a,c
    and %00000111
    ld (hl),a

    inc c
    jp nz,-
; - - - -2*256=512b

    ; todo set star states to 0.

    ret
;; ===================================
    .struct starf
       ; space state
        y db
        x db
        z db
      ; draw state
        flags db  ; 1 drawn 2started
        bmpos dw ; in bm

    .endst

starfield3d_tms_frame:

    ld ix,dd_starbase
    ld c,stf_nbStars
        ; loop per star
	ld bc,_sizeof_starf ; ad vec
-:
    ld a,(ix+starf.z)

    ;inc


    ; - - next
	add ix,bc
    dec c
    jp nz,-

    ret
; - - - -
projtab:
	.incbin res/projection128.cbin

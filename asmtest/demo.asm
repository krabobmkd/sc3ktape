; - - -source for demo screen with gfx and sound and keyb/joy inputs
.include sc3k.i
.include basicinit.basicoffsets.i

.define do_music 1
.define do_saveBasicLV3RamState 1

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


.define freeramstart1 (blv3start+bin_end-binstart+8)

;The heaven of memcpy-like optimization in Z80 is the stack. If you have destination fixed, for example, you do like:
;ld   sp,src
;pop  hl
;ld   [dest+0],hl
;pop  hl
;ld   [dest+2],hl
; 14 clocks instead of 21 for ldir/lddr


	.org blv3start
binstart: ; used to get size
main:

	call vdp_setBlank

	; - -  clean ram
	ld a,0
	ld hl,freeramstart
	ld bc,dd_demodataend-freeramstart1
	call memset
	; - -- save basic ram states
	.ifdef do_saveBasicLV3RamState
	call basicdump_save
	.endif

	.ifdef do_music
	call	PSGInit
	ld		hl,musicdata
	call	PSGPlay
	.endif

	; init scrolltext
	ld hl,scrolltext
	ld (dd_textptrstart),hl
	ld (dd_textptr),hl


;re?	call vdp_setVerticalNames

    ;ld hl,bmcdata ;skull_bm_cdata
    ld hl,logo_bm_cdata
    call decomp_to_dvp

   ;ld hl,clcdata ;
	ld hl,logo_cl_cdata
    call decomp_to_dvp

	call vdpcopy_color_line

	call starfield3d_init

	call vdp_set_screen2

mainloop:
	waitVBlank

	; should do all VDP copies the quickliest possible, here after waitVBlank.

	call vdpcopy_from_linear_line
	call starfield3d_frame_vdp

	.ifdef do_music
		call PSGFrame
	.endif

	; should do ram stuff here
	call scroll_line_ram
	call starfield3d_frame_ram


; - -  test inputs for quitting
	CheckKb_space
	jp z,mainend
	CheckKb_break
	jp z,mainend

    jp mainloop
mainend:

	.ifdef do_music
		call PSGStop
	.endif

	; - -- restore basic ram states
	.ifdef do_saveBasicLV3RamState
	call basicdump_load
	.endif
	ret
; - - - end of main


vdp_setBlank:
	di
		ld hl,VDPRegs_blank
		ld b,VDPRegs_blank_end-VDPRegs_blank
		ld c,VDPControl
		otir
	ei
	ret
vdp_set_screen2:
	di
		ld hl,VDPRegs_screen2
		ld b,VDPRegs_screen2_end-VDPRegs_screen2
		ld c,VDPControl
		otir
	ei
	ret
	.ifdef do_verticalNAmes
vdp_setVerticalNames:
	; write to names
	SetVDPAddress $3800 -1
	ld c,VDPData ; used by out as port id.

	ld d,3
---
	ld b,0
--:
	ld a,b
	ld e,32
-:
	out (c),a
	add a,8
	dec e
	jp nz,-

	inc b
	ld a,8
	cp b
	jp nz,--

	dec d
	jp nz,---

	ret
	.endif
; - - - - -
;reg0: 2:mode2
;reg1
; 1110 0000 <-graphics (16k,enabledislay,enableframeinterupt,9918mode1)
;						(mode3,noeffect,large16x16sprite,stretchd sprites )
; 1110 0010
; reg2 name table base adress 4b
; reg2 sur elevato action switch 0E/0F pour du double buff ?

;reg7 overscan & backdrop color: normal si tjrs different.

;meka for screen2: 02 E0 0E FF 03 76 03 05   00 00 FF
; meka bank panic: 02 E2 0E FF 03 76 03 00   00 00 FF
;                                       F0
;                           FF 03 7F   <-exerion chaneg reg5(sprite attribs base adr)
;elevatoraction: 02 E2 screen title, 02 82 when screen switch (switch off display & screen interupt)
VDPRegs_screen2:
	.db $02,$80
	.db $E2,$81  ; 1110 0010     16k,enable display,enable vert. initerupt, large sprites

	.db $0E,$82
	.db $ff,$83
	.db $03,$84
	.db $76,$85
	.db $03,$86

	.db $11,$87 ; bg colors
;.db $14,$80,$00,$81,$ff,$82,$ff,$85,$ff,$86,$ff,$87,$00,$88,$00,$89,$ff,$8a
VDPRegs_screen2_end:
VDPRegs_blank:
	.db $00,$80
	.db $80,$81 ; display off
VDPRegs_blank_end:
; Fills a memory area with a certain value.
; a = contains the fill value.
; hl = address to fill
; bc = size
memset:
    ld (hl),a
    ld e,l
    ld d,h
    inc de
    dec bc
    ldir
    ret
; - - - - - -- - - - -
.ifdef do_saveBasicLV3RamState
basicdump_save:
	ld ix,basiclevel3dumpadr
	ld de,dd_basicdump
	ld a,(basiclevel3dumpadrEnd-basiclevel3dumpadr)/3
	ld b,0 ; because ldir
-:
	ld l,(ix+0)
	ld h,(ix+1)
	ld c,(ix+2)
	inc ix
	inc ix
	inc ix

	ldir ; Repeats LDI (LD (DE),(HL), then increments DE, HL, and decrements BC) until BC=0. N

	dec a
	jp nz,-

	ret
basicdump_load:

; Fills a memory area with a certain value.
; a = contains the fill value.
; hl = address to fill
; bc = size
	; first there is 0 all around
	ld a,0
	ld hl,$8000
	ld bc,$0800 ; 2kb
	call memset
	; then put bsic variables backs:
	ld ix,basiclevel3dumpadr
	ld hl,dd_basicdump
	ld a,(basiclevel3dumpadrEnd-basiclevel3dumpadr)/3
-:
	ld e,(ix+0)
	ld d,(ix+1)
	ld c,(ix+2)
	inc ix
	inc ix
	inc ix

	ldir ; Repeats LDI (LD (DE),(HL), then increments DE, HL, and decrements BC) until BC=0. N

	dec a
	jp nz,-

ret

; basic level3 $8000 - $8800 dump size to free 2kb
.define blds_a 24
.define blds_b 32
.define blds_c 64
.define blds_d 16
.define blds_e 2
.define blds_f 16+2

basiclevel3dumpadr:
	.dw $8000
	.db	blds_a
	.dw $8160
	.db blds_b	; basic start pointers
	.dw $8270
	.db blds_c
	.dw $84a0
	.db blds_d
	.dw $85a0+3
	.db blds_e
	.dw $86e6
	.db blds_f
basiclevel3dumpadrEnd:

.define blv3_basic_dump_size (blds_a+blds_b+blds_c+blds_d+blds_e+blds_f) ; ~150b copied to make place to 2.5kb
.endif
; - - - - - - - - - - -- - - - - - -- - - - -

	; gfx decompression
	.include decomp.asm

	.include linescroll.asm
	.include starfield3d.asm
	; music player source
	.ifdef do_music
	.include PSGLib_blv3.asm
	.endif
	; graphics
	.include krb_mkd5.sc2.asm
	; music data
	.ifdef do_music
musicdata:
		.incbin SONG_FILE
musicdata_end:
	.endif
	; note there should a charset in lv3b rom !
ext_charset:
	.incbin CHARSET
scrolltext:
	.db "... HELLO PEOPLE, HOW DO YOU THINK THIS IS GOING ? .... THIS IS AN OLDSCHOOL SCROLL !!! "
	.db "IT IS EVEN MORE BADASS WHEN IT RUNS ON A 1983 SEGA MACHINE THAT IS NOT MEANT TO SCROLL AT ALL IN THE FIRST PLACE !!! "
	.db "YES THIS IS ALL 100% Z80 SUFFERING. "
	.db "BIG GREETINGS TO ... G.E.R.A.N.I.U.M. .... FOR BEING THE FIRST DEMOGROUP TO EVER RELEASE A SEGA SC3000 TAPE DEMO IN ASSEMBLER (2019), "
	.db "WHICH I SUPPOSE IMPLIES WE, THE ALMIGHTY -MKD-, ARE SECOND ."
	.db "   ... ALL THIS IS ON HTTP://GITHUB.COM/KRABOBMKD ANYWAY .... SCROLL RESTART   "
	.db 0
; include any ressource here
	; this marks the end and of our binary
	; with basicLV3, it is followed by a short vars section, then actual free memory
bin_end:
; inclusive end ram values, for info:
; actual ram for BasicLevelIIIA: 10239 dec. (or +2kb ($800) ?)
;.define freeram_blv3_A_end $bfff
; actual ram for BasicLevelIIIB: 26623 dec.
;.define freeram_blv3_B_end $ffff

; - - - - definition of variables and buffers in free ram, which is mapped after bin, but is not part of bin:

.define freeramstart (blv3start+bin_end-binstart+8)

	; watch out, not zeroed at start !
	.enum freeramstart export
		; all global vars should be defined there:
		dd_wtempx	dw
		dd_wtempy	dw

		; - - - line scroll vars
		dd_linebuffer ds 32*7 ; actual linear bitmap of scroll line in ram
		dd_newchar ds 7 ; bitmap of incoming char, 8: height of chars
		dd_newcharcount db ; each 8 pixels give new char
		dd_textptrstart	dw ; char index in text to reset at start
		dd_textptr	dw ; char index in text
		; - - - - starfield vars
		;dd_mainz db
		dd_starpos ds (stf_nbStars*stf_size) ; y x pointer pos, 4*nbstar
		;triple buffer rolling to display a bit of latency
		;dd_starbmA ds (stf_nbStars+1)*3 ; bm write pos base A
		;dd_starbmB ds (stf_nbStars+1)*3 ; bm write pos base B
		;dd_starbmC ds (stf_nbStars+1)*3 ; bm write pos base B
		;dd_starbm1 dw
		;dd_starbm2 dw
		;dd_starbm3 dw

		.ifdef do_saveBasicLV3RamState
		dd_basicdump ds blv3_basic_dump_size
		.endif

		; - - - -
		;keep at end to get size:
		dd_demodataend db
	.ende
	.ifdef do_music
	; struct of PSGLib, append to dd_demodataend
	.include PSGLib_blv3_ramvars.i
	.endif

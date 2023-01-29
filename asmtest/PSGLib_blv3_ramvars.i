

;.ramsection "PSGlib variables" slot 3
 .enum dd_demodataend export                ; PSGlib variables location in RAM
  ; fundamental vars
  PSGMusicStatus             db    ; are we playing a background music?
  PSGMusicStart              dw    ; the pointer to the beginning of music
  PSGMusicPointer            dw    ; the pointer to the current
  PSGMusicLoopPoint          dw    ; the pointer to the loop begin
  PSGMusicSkipFrames         db    ; the frames we need to skip
  PSGLoopFlag                db    ; the tune should loop forever or not (flag)
  PSGLoopCounter             db    ; how many times the tune should loop
  PSGMusicLastLatch          db    ; the last PSG music latch

  ; decompression vars
  PSGMusicSubstringLen       db    ; lenght of the substring we are playing
  PSGMusicSubstringRetAddr   dw    ; return to this address when substring is over

  ; command buffers
  PSGChan0Volume             db       ; the volume for channel 0
  PSGChan1Volume             db       ; the volume for channel 1
  PSGChan2Volume             db       ; the volume for channel 2
  PSGChan3Volume             db       ; the volume for channel 3
  PSGChan2LowTone            db       ; the low tone bits for channels 2
  PSGChan3LowTone            db       ; the low tone bits for channels 3
  PSGChan2HighTone           db       ; the high tone bits for channel 2

  PSGMusicVolumeAttenuation  db       ; the volume attenuation applied to the tune (0-15)

  ; ******* SFX *************

  ; flags for channels 2-3 access
  PSGChannel2SFX             db       ; !0 means channel 2 is allocated to SFX
  PSGChannel3SFX             db       ; !0 means channel 3 is allocated to SFX

  ; command buffers for SFX
  PSGSFXChan2Volume          db       ; the volume for channel 2
  PSGSFXChan3Volume          db       ; the volume for channel 3

  ; fundamental vars for SFX
  PSGSFXStatus               db       ; are we playing a SFX?
  PSGSFXStart                dw       ; the pointer to the beginning of SFX
  PSGSFXPointer              dw       ; the pointer to the current address
  PSGSFXLoopPoint            dw       ; the pointer to the loop begin
  PSGSFXSkipFrames           db       ; the frames we need to skip
  PSGSFXLoopFlag             db       ; the SFX should loop or not (flag)

  ; decompression vars for SFX
  PSGSFXSubstringLen         db       ; lenght of the substring we are playing
  PSGSFXSubstringRetAddr     dw       ; return to this address when substring is over
;.ends
	.ende                    ; in case you want to use .enum instead of .ramsection

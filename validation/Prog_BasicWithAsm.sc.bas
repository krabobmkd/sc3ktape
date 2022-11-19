# vas-y !!!
    REM SEGA ASSEMBLY PROGRAMMING
    REM woot
    print hex$(peek(%BINREF));hex$(peek(%BINREF+1))

endloop:
    goto endloop

    incbin testbin.bin


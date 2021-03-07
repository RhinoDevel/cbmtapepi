
; 2021.03.03
;
; marcel timm, rhinodevel

; cbm pet

; *********************
; *** configuration ***
; *********************

bas_ver = 4 ; pet basic version to assemble for. can be set to 1, 2 or 4,
             ; set to 20 for vic 20 assembly.

tom_install = 0 ; 1 = install to top of memory, 0 = install in tape buffers
                ;     (which is possible for pet machines, only).

; *********************

Incasm "basic.asm"
Incasm "defines.asm"

if tom_install = 1
    Incasm "topofmem.asm"
else
    Incasm "tapebuf.asm"
endif

Incasm "install.asm"
Incasm "wedge.asm"
Incasm "main.asm"
Incasm "sendbyte.asm"
Incasm "readbyte.asm"
Incasm "deb.asm"

Generateto cbmtapepi.prg

; 2021.03.03
;
; marcel timm, rhinodevel

; cbm pet

Incasm "basic.asm"
Incasm "defines.asm"

; choose one file to either install into top of memory via basic loader,
; or to install into tape buffers via directly loading into tape buffers:
;
;Incasm "topofmem.asm" ; TODO: currently installs into tape buffers, too.
Incasm "tapebuf.asm"

Incasm "install.asm"
Incasm "wedge.asm"
Incasm "main.asm"
Incasm "sendbyte.asm"
Incasm "readbyte.asm"
Incasm "deb.asm"

Generateto cbmtapepi.prg
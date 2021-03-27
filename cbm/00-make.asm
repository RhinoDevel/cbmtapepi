
; 2021.03.03
;
; marcel timm, rhinodevel

; *********************
; *** configuration ***
; *********************

bas_ver = 4 ; pet basic version to assemble for. can be set to 1, 2 or 4,
            ; set to 20 for vic 20 assembly, 64 for c64 assembly.

tom_install = 0 ; 0 = install in tape buffers
                ;     (which is possible for pet machines, only).
                ; 1 = install to top of memory.
                ; 2 = install at top of highest free memory (c64, only).
                ;
                ;     ALWAYS TO-DO FOR install at top of highest free memory:
                ;     *******************************************************
                ;     1) Check, if relocate address is correct (must be
                ;        hard-coded, as assembler does not seem to support a
                ;        label with Relocate..).
                ;     2) Uncomment Relocate (must be done, because assembler
                ;        seems to ignore conditional assembly with Relocate..).
                ;
                ;     ALWAYS TO-DO FOR other install options:
                ;     ***************************************
                ;     1) Comment-out Relocate (see comment, above).

; *********************

Incasm "01-basic.asm"
Incasm "02-defines.asm"

if tom_install = 0
    Incasm "03-tapebuf.asm"
endif
if tom_install = 1
    Incasm "03-topofmem.asm"
endif
if tom_install = 2
    Incasm "03-topofree.asm"
    ;Relocate $ced2;cpy_addr ; see ALWAYS TO-DO list, above!
endif

Incasm "04-install.asm"
Incasm "05-wedge.asm"
Incasm "06-main.asm"
Incasm "07-sendbyte.asm"
Incasm "08-readbyte.asm"
Incasm "09-debug.asm"

Generateto cbmtapepi.prg

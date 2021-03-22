
; 2021.03.03
;
; marcel timm, rhinodevel

; cbm pet

; ***********************
; *** wedge installer ***
; ***********************

         lda #$4c       ; jmp
         sta chrget
install_low
         lda #<wedge
         sta chrget + 1
install_high
         lda #>wedge
         sta chrget + 2

         sei

         ; toggle logic level all ~250us (see kernel_main.c):
         ;
         ; - assuming 1mhz cpu frequency <=> 1us per cpu cycle.

         ;lda #10        ;   10 * ~64ms = ~640ms for server to detect signal.
         ;sta temp0      ;

starty   ldy #$ff       ;   255 * ~252us = ~64ms for server to detect signal.

startlvl lda cas_wrt    ;   4! toggle level on write line to generage frequency
         eor #ackmask   ;   2! signaling to server that fastmode wedge is
         sta cas_wrt    ;   4! installed at cbm.

         ; 5 * x + 17 = 250 => x = (250 - 17) / 5 = ~47 => ~252us
         ;
         ldx #47        ;   2! wait to get frequency wanted.
keep_lvl dex            ;   2:
         bne keep_lvl   ; 3/2!

         dey            ;   2!
         bne startlvl   ;   3!

         ;dec temp0
         ;bne starty

         cli

if tom_install = 1
         rts
else
         jmp new ; repairs start-of-variables pointer, etc., these were changed
                 ; by loading into tape buffer(-s).
endif


; 2021.03.02
;
; marcel timm, rhinodevel

; cbm pet

; ********************************************************
; *** read a byte into register a and memory at temp0. ***
; ********************************************************
; *** modifies registers a, x and memory at temp0.     ***
; ********************************************************

readbyte ldx #8         ; (read bit) counter.

readwait bit cas_read   ; wait for data-ready toggling (writes bit 7 to n flag).
         bpl readwait   ; branch, if n is 0 ("positive").

         bit cas_read-1 ; resets "toggle" bit by read operation (see pia doc.).

         lda cas_sens   ; load actual data (bit 4) into c flag.
         and #indamask  ; sets z flag to 1, if bit 4 is 0.
         clc            ;
         beq readadd    ; bit read is zero.
         sec            ;

readadd  ror temp0      ; put read bit from c flag into byte buffer.

         lda cas_wrt    ; acknowledge data bit ("toggle" data-req. line level).
         eor #ackmask   ; toggle output bit.
         sta cas_wrt    ;

         dex
         bne readwait   ; last bit read?

         lda temp0
         rts            ; read byte is in register a and in memory at temp0,
                        ; x is 0, here!

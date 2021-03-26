
; 2021.03.02
;
; marcel timm, rhinodevel

; cbm pet

; ********************************************************
; *** send a byte from register y.                     ***
; ********************************************************
; *** modifies registers a, y and memory at temp0.     ***
; ********************************************************

sendbyte sty temp0      ; byte buffer during send.
         ldy #8         ; (send bit) counter.

sendloop lsr temp0      ; sends current bit to c flag.

         lda cas_wrt    ; (does not change c flag)
         and #oudmaskn  ; set bit to zero to send 0/low (does not change c fl.).
         bcc senddata
         ora #oudmask   ; set bit to one to send 1/high.
senddata sta cas_wrt    ; set data bit.

         lda cas_moto   ; motor signal toggle.
         eor #ordmask   ;
         sta cas_moto   ;

sendwait lda cas_read
         and #inackmask
         beq sendwait
         ;
         ; this could be used alternatively for pet machines:
         ;
         ;bit cas_read   ; wait for data-ack. high-low (writes bit 7 to n flag).
         ;bpl sendwait   ; branch, if n is 0 ("positive").

         bit cas_read_reset ; resets "toggle" bit by read operation (see docs).

         dey
         bne sendloop   ; last bit read?

         rts            ; y is 0, here!


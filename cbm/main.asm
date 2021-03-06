
; 2021.03.02
;
; marcel timm, rhinodevel

; cbm pet

; ************** 
; *** macros ***
; **************

defm     call_sendbyte
if tom_install = 1
         byte $20 ; jsr
         byte $00 ; marker (together with preceding jsr).
         byte tom_send_offset
else
         jsr sendbyte
endif
         endm

defm     call_readbyte
if tom_install = 1
         byte $20 ; jsr
         byte $00 ; marker (together with preceding jsr).
         byte tom_read_offset
else
         jsr readbyte
endif
         endm

; ************
; *** main ***
; ************

         ; (byte at temp0 can be reused from here on)

main     sei

         jmp exit ; TODO: debugging!

         ; commented-out, because directly using sovptr during send and
         ; retrieve:
         ;
         ;lda sovptr
         ;sta lim
         ;lda sovptr + 1
         ;sta lim + 1

         lda #0
         sta addr
         sta addr + 1
         
         tax ; (for sending command string, below)

         lda str
         cmp #sav_char
         bne addrlim_rdy

         lda sobptr
         sta addr
         lda sobptr + 1
         sta addr + 1
addrlim_rdy

; >>> send bytes: <<<

         ; motor signal must already be low or on its way to low, here.      

         bit cas_read-1 ; maybe not needed: makes sure that flag raised by
                        ;                   high-to-low on read line is not
                        ;                   raised (see pia documentation).

         ;ldx #0        ; send command string.
strnext  ldy str,x
         call_sendbyte
         inx
         cpx #str_len
         bne strnext

         ldy addr       ; send address.
         call_sendbyte
         ldy addr + 1
         call_sendbyte

         lda addr       ; address is zero => no limit and payload to send.
         bne send_lim
         lda addr + 1
         beq retrieve

send_lim ldy sovptr;lim ; send first address above payload ("limit").
         call_sendbyte
         ldy sovptr + 1;lim + 1
         call_sendbyte

s_next   ;ldy #0         ; (y is always 0 after sendbyte) ; send payload.
         lda (addr),y
         tay
         call_sendbyte
         inc addr       ; increment to next (read) address.
         bne s_finchk
         inc addr + 1
s_finchk lda addr       ; check, if end is reached.
         cmp sovptr;lim
         bne s_next
         lda addr + 1
         cmp sovptr + 1;lim + 1
         bne s_next

; >>> retrieve bytes: <<<

; expected values at this point:
;
; cas_sens = data input.
; cas_read = data-ready input, don't care about level, just care about
;            high -> low change.
;
; cas_wrt  = data-ack. output. current level was decided by sending done, above.

retrieve 
         call_readbyte  ; read address.
         sta addr
         call_readbyte
         sta addr + 1
         
         ; [cmp #0 is not necessary, because of last lda temp0 in readbyte()]
         ;
         ;lda addr + 1
         ;cmp #0        ; exit, if addr. is 0.
         bne read_lim
         lda addr
         beq exit      ; todo: overdone and maybe unwanted (see label)!

read_lim
         call_readbyte  ; read payload "limit" (first addr. above payload).
         sta sovptr;lim
         call_readbyte
         sta sovptr + 1;lim + 1

r_next
         call_readbyte   ; retrieve payload.
         ;ldx #0        ; [x is always 0 after readbyte()]
         sta (addr,x)   ; store byte at current address.
         inc addr       ; increment to next (write) address.
         bne r_finchk
         inc addr + 1
r_finchk lda addr       ; check, if end is reached.
         cmp sovptr;lim
         bne r_next
         lda addr + 1
         cmp sovptr + 1;lim + 1
         bne r_next

         ;;lda addr + 1
         ;sta sovptr + 1 ; set basic variables start pointer to behind loaded
         ;lda addr       ; payload.
         ;sta sovptr     ;

exit     cli

         jsr rstxclr    ; equals preparations after basic load at
         jsr rechain    ; $c430/$c439/$b4ad.

         jmp ready

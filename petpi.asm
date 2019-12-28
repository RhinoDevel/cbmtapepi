
; 2017.09.13
;
; marcel timm, rhinodevel

; cbm pet

; configured for basic 2.0 / rom v3.
;
; can be reconfigured for basic 1.0 / rom v2 by replacing values with following
; values in comments (if there are no commented-out values following, these
; addresses are equal).

*=826 ; tape buf. #2 used (192 bytes).
;*=634 ; tape buf. #1 & #2 used (192+192=384 bytes), load via basic loader prg.

; ---------------
; system pointers
; ---------------

varstptr = 42;124 ; pointer to start of basic variables.
varenptr = 44;126 ; pointer to end of basic variables.

; -----------
; "constants"
; -----------

tapbufin = $bb;$271 ; tape buf. #1 & #2 indices to next char (2 bytes).
adptr    = 15;6 ; term. width & lim. for scanning src. columns (2 unused bytes).

; using tape #1 port for transfer:

cas_sense = $e810 ; bit 4.
;
; pia 1, port a (59408, tested => correct, 5v/1 when no key pressed or
;                unconnected, "0v"/0 when key pressed).

cas_read = $e811 ; bit 7 is high-to-low flag. pia 1, control register a (59409).

cas_write = $e840 ; bit 3
;
; via, port b (59456, tested => correct, 5v for 1, 0v output for 0).

out_req  = cas_write
in_ready = cas_read
in_data  = cas_sense
reqmask  = 8 ; bit 3.
reqmaskn = 1 not reqmask ; (val. before not operator does not seem to matter)
datamask = $10 ; bit 4.

; ---------
; functions
; ---------

; ************
; *** main ***
; ************

         ;cld
         sei

; expected values at this point:
;
; cas_write/out_req = output, default level will be set to low.
; cas_read/in_ready = input, don't care about level, just care about high -> low
;                     change.
; cas_sense/in_data = input.

         lda out_req   ; request-data line default level is low.
         and #reqmaskn
         sta out_req

         jsr readbyte  ; read start address.
         sta adptr
         jsr readbyte
         sta adptr+1

         jsr readbyte  ; read payload byte count.
         sta tapbufin
         jsr readbyte
         sta tapbufin+1

nextpl   jsr readbyte  ; read byte.

         ldy #0        ;
         sta (adptr),y ; store byte at current address.

         inc adptr
         bne decle
         inc adptr+1

decle    lda tapbufin
         cmp #1
         bne dodecle
         lda tapbufin+1      ;low byte is 1
         beq readdone  ;read done,if high byte is 0
dodecle  dec tapbufin        ;read is not done
         lda tapbufin
         cmp #$ff
         bne nextpl
         dec tapbufin+1      ;decrement high byte,too
         jmp nextpl

readdone lda adptr+1    ; set basic variables start and end pointers to behind
         sta varstptr+1 ; loaded prg.
         sta varenptr+1 ;
         lda adptr      ;
         sta varstptr   ;
         sta varenptr   ;

         cli
         rts

; **************************************
; *** read a byte into register a    ***
; **************************************
; *** modifies registers a, x and y. ***
; **************************************

readbyte ldy #0         ; byte buffer during read.
         ldx #0         ; (read bit) counter.

readloop txa            ; req. next data bit ("toggle" data-request line level).
         and #1
         beq toghigh
         lda out_req    ; toggle output to low.
         and #reqmaskn
         jmp togdo
toghigh  lda out_req    ; toggle out_req output to high.
         ora #reqmask
togdo    sta out_req    ; [does not work in vice (v3.1)]

readwait bit in_ready   ; wait for data-ready toggling (writes bit 7 to n flag).
         bpl readwait   ; branch, if n is 0 ("positive").

         bit in_ready-1 ; resets "toggle" bit by read operation (see pia doc.).

         lda in_data    ; load actual data (bit 4) into c flag.
         clc            ;
         and #datamask  ; sets z flag to 1, if bit 4 is 0.
         beq readadd    ; bit read is zero.
         sec            ;

readadd  tya            ; put read bit from c flag into byte buffer.
         ror            ;
         tay            ;

         inx
         cpx #8         ; last bit read?
         bne readloop

         tya            ; get byte read into accumulator.
         rts

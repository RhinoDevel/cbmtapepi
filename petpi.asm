
; 2017.09.13
;
; marcel timm, rhinodevel

; cbm pet

; configured for basic 2.0 / rom v3.
; can be reconfigured for basic 1.0 / rom v2 by
; replacing values with following values in comments
; (if there are no commented-out values following,
; these addresses are equal).

*=32384
;*=634 ; tape buf. #1 & #2 used (192+192=384 bytes), load via basic loader prg.

; -------------------
; system sub routines
; -------------------

crlf     = $c9e2;$c9d2 <- basic 1.0 / rom v2 value.
wrt      = $ffd2

; --------------
; basic commands
; --------------

run      = $c785;$c775 ; basic run.

; ---------------
; system pointers
; ---------------

varstptr = 42;124 ; pointer to start of basic variables.

; -----------
; "constants"
; -----------

chr_0    = $30
chr_a    = $41
chr_spc  = $20

cursor   = $c4;$e0

tapbufin = $bb;$271 ; tape buf. #1 & #2 indices to next char (2 bytes).
adptr    = 15;6 ; term. width & lim. for scanning src. columns (2 unused bytes).

; using tape #1 port for transfer:

cas_sense = $e810 ; bit 4.
;
; pia 1, port a (59408, tested => correct, 5v/1 when no key pressed or
;                unconnected, "0v"/0 when key pressed).

cas_read = $e811 ; bit 0. pia 1, control register a (59409)

cas_write = $e840 ; bit 3
;
; via, port b (59456, tested => correct, 5v for 1, 0v output for 0).

defbasic = $401 ; default start addr. of basic prg.

out_req  = cas_write
in_ready = cas_read
in_data  = cas_sense

; ---------
; functions
; ---------

; ************
; *** main ***
; ************

         ;cld

; expected values at this point:
;
; cas_write/out_req = output.
; cas_read/in_ready = input, don't care about level, just care about HIGH -> LOW
;                     change.
; cas_sense/in_data = input.

         lda out_req
         lsr a
         lsr a
         lsr a
         and #1
         sta tapbufin

         jsr readbyte  ; read start address.
         sta adptr     ; store for transfer.
         sta loadadr   ; store for later autostart.
         jsr readbyte
         sta adptr+1
         sta loadadr+1

         ;lda adptr+1  ; print start address.
         jsr printby
         lda adptr
         jsr printby

         lda #chr_spc
         jsr wrt

         jsr readbyte  ; read payload byte count.
         sta le
         jsr readbyte
         sta le+1

         ;lda le+1     ; print payload byte count.
         jsr printby
         lda le
         jsr printby

nextpl   jsr readbyte  ; read byte.

         ldy #0        ; store byte
         sta (adptr),y ; at current address.

         inc adptr
         bne decle
         inc adptr+1

decle    lda le
         cmp #1
         bne dodecle
         lda le+1      ;low byte is 1
         beq readdone  ;read done,if high byte is 0
dodecle  dec le        ;read is not done
         lda le
         cmp #$ff
         bne nextpl
         dec le+1      ;decrement high byte,too
         jmp nextpl

readdone lda loadadr    ; decide, if run shall be exec. (based on start addr.).
         cmp #<defbasic ;
         bne exit       ;
         lda loadadr+1  ;
         cmp #>defbasic ;
         bne exit       ;

         lda adptr+1    ; set basic variables start pointer to behind loaded
         sta varstptr+1 ; prg.
         lda adptr      ;
         sta varstptr   ;

         jsr crlf

         lda #0         ; this actually
         jmp run        ; is ok (checked stack pointer values).

exit     rts

; ****************************************
; *** "toggle" write based on tapbufin ***
; ****************************************

togout   lda tapbufin ; "toggle" depending on tapbufin.
         beq toghigh
         dec tapbufin ; toggle output to low.
         lda out_req
         and #247
         jmp togdo
toghigh  inc tapbufin ; toggle out_req output to high.
         lda out_req
         ora #8
togdo    sta out_req ; [does not work in vice (v3.1)]
         rts

; ************************************
; *** read a byte into accumulator ***
; ************************************

readbyte sei
         ldy #0         ; byte buffer during read.
         ldx #1         ; to hold 2^exp.

readloop jsr togout     ; request next data bit.

readwait bit in_ready   ; wait for data-ready toggling.
         bpl readwait   ;
         bit in_ready-1 ; resets toggle flag.

         lda in_data       ; load actual data (bit 4).
         lsr a
         lsr a
         lsr a
         lsr a
         and #1

         beq readnext   ; bit read is zero.
         stx tapbufin+1 ; bit read is one, add to byte (buffer).
         tya            ; get current byte buffer content.
         ora tapbufin+1 ; "add" current bit read.
         tay            ; save into byte buffer.
readnext txa            ; get next 2^exp.
         asl
         tax
         cpx #0         ; last bit read?
         bne readloop
         tya            ; get byte read into accumulator.
         cli
         rts

; *********************************************************
; *** print "hexadigit" (hex.0-f) stored in accumulator ***
; *********************************************************

printhd  and #$0f      ;ignore left 4 bits
         cmp #$0a
         bcc printd
         clc           ;more or equal $0a - a to f
         adc #chr_a-$0a
         bcc print
printd   ;clc           ;less than $0a - 0 to 9
         adc #chr_0
print    jsr wrt
         rts

; ******************************************************
; *** print byte in accumulator as hexadecimal value ***
; ******************************************************

printby  pha
prbloop  lsr a
         lsr a
         lsr a
         lsr a
         jsr printhd
         pla
         jsr printhd
         rts

; ---------
; variables
; ---------

le       byte 0, 0 ; count of payload bytes.
crsrbuf  byte 0, 0, 0
loadadr  byte 0, 0 ; hold start address of loaded prg.

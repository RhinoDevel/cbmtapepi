
; 2017.09.13
;
; marcel timm, rhinodevel

; cbm pet

; configured for basic 2.0 / rom v3.
; can be reconfigured for basic 1.0 / rom v2 by
; replacing values with following values in comments
; (if there are no commented-out values following,
; these addresses are equal).

*=634 ;tape buf. #1 and #2 used (192+192=384 bytes), load via basic loader prg.

; -------------------
; system sub routines
; -------------------

crlf     = $c9e2          ;$c9d2 <- basic 1.0 / rom v2 value
wrt      = $ffd2
get      = $ffe4
clrscr   = $e229          ;$e236

; --------------
; basic commands
; --------------

run      = $c785          ;$c775 ;basic run

; ---------------
; system pointers
; ---------------

varstptr = 42          ;124 ;pointer to start of basic variables

; -----------
; "constants"
; -----------

chr_stop = 3
chr_0    = $30
chr_a    = $41
chr_spc  = $20

tapbufin = $bb          ;$271 ;tape buf. #1 & #2 indices to next char (2 bytes)
cursor   = $c4          ;$e0

; using tape #1 port for transfer:

cas_sense = $e810          ;bit 4
;
;pia 1, port a (59408, tested => correct, 5v/1 when no key pressed or
;               unconnected, "0v"/0 when key pressed)

cas_read = $e811          ;bit 0
;
;pia 1, control register a (59409)

cas_write = $e840          ;bit 3
;
;via, port b (59456, tested => correct, 5v for 1, 0v output for 0)

defbasic = $401          ;default start addr. of basic prg

adptr    = 15          ;6 ;unused terminal & src. width

; ---------
; functions
; ---------

; ************
; *** main ***
; ************

         cld

         jsr clrscr

                       ; expected values at this point:
                       ;
                       ; cas_write/read-ack   = output, set to high (cbm default)
                       ; cas_read/data        = input, don't care about level
                       ; cas_sense/data-ready = input, set to low (done by pi, not cbm default)

         lda #1        ;make sure that initial data-ready signal
         sta drmo+1    ;to expect on cas_sense is set to one

                       ; ask for data:
                       ;
         jsr out2low   ;set read-ack on cas_write to low

         jsr readbyte  ;read start address
         sta adptr     ;store for transfer
         sta loadadr   ;store for later autostart
         jsr readbyte
         sta adptr+1
         sta loadadr+1

                       ;lda adptr+1    ;print start address
         jsr printby
         lda adptr
         jsr printby

         lda #chr_spc
         jsr wrt

         jsr readbyte  ;read payload byte count
         sta le
         jsr readbyte
         sta le+1

                       ;lda le+1       ;print payload byte count
         jsr printby
         lda le
         jsr printby
         jsr crlf

keywait  jsr get       ;wait for user key press
         beq keywait
         cmp #chr_stop
         bne cursave   ;exit,if run/stop was pressed
break    jsr out2low   ;return with read-ack on cas_write set to low
         rts

cursave  lda cursor    ;remember cursor position for progress updates
         sta crsrbuf
         lda cursor+1
         sta crsrbuf+1
         lda cursor+2
         sta crsrbuf+2

nextpl   lda crsrbuf   ;reset cursor position for progress update on screen
         sta cursor
         lda crsrbuf+1
         sta cursor+1
         lda crsrbuf+2
         sta cursor+2

         lda adptr+1   ;print current byte address
         jsr printby
         lda adptr
         jsr printby

         lda #chr_spc
         jsr wrt

         lda le+1      ;print current count of bytes left
         jsr printby
         lda le
         jsr printby

         jsr readbyte  ;read byte
         ldy #0        ;store byte at current address
         sta (adptr),y

         lda #chr_spc
         jsr wrt
                       ;ldy #0
         lda (adptr),y ;print byte read
         jsr printby

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

readdone jsr crlf

         jsr out2low

         lda loadadr   ;decide,if basic or asm prg loaded
         cmp #<defbasic;(decision based on start address, only..)
         bne runasm
         lda loadadr+1
         cmp #>defbasic
         bne runasm

         lda adptr+1   ;set basic variables start pointer to behind loaded prg
         sta varstptr+1
         lda adptr
         sta varstptr

         jsr crlf

         lda #0        ;this actually
         jmp run       ;is ok (checked stack pointer values)

runasm   jmp (loadadr)

; ****************************************
; *** "toggle" write based on tapbufin ***
; ****************************************

togout   lda tapbufin  ;"toggle" depending on tapbufin
         beq toghigh
         dec tapbufin  ;toggle output to low
         lda cas_write
         and #247
         jmp togdo
toghigh  inc tapbufin  ;toggle output to high
         lda cas_write
         ora #8
togdo    sta cas_write ;does not work in vice (v3.1)!
         rts

; ************************
; *** set write to low ***
; ************************

out2low  lda #1
         sta tapbufin
         jsr togout
         rts

; ************************************
; *** read a byte into accumulator ***
; ***                              ***
; *** must be used by main, only!  ***
; ************************************

readbyte ldy #0        ;byte buffer during read
         ldx #1        ;to hold 2^exp
readloop jsr get       ;let user be able to break execution with run/stop key
         beq readcont
         cmp #chr_stop
         bne readcont  ;exit,if run/stop was pressed

         pla           ;hard-coded break -
         pla           ;function usable by main only,
         jmp break     ;because of this..

readcont lda cas_sense ;wait for data-ready signal
         and #16       ;data-ready line
         lsr a
         lsr a
         lsr a
         lsr a
drmo     cmp #0        ;this value will be toggled between 0 and 1 in-place.
         bne readloop

         eor #1        ;toggle next data-ready val.to expect
         sta drmo+1
         lda cas_read
         and #1        ;data line

         beq readnext  ;bit read is zero
         stx tapbufin+1;bit read is one, add to byte (buffer)
         tya           ;get current byte buffer content
         ora tapbufin+1;"add" current bit read
         tay           ;save into byte buffer
readnext txa           ;get next 2^exp
         asl
         tax
         jsr togout    ;acknowledge
         cpx #0        ;last bit read?
         bne readloop
         tya           ;get byte read into accumulator
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
printd                 ;clc           ;less than $0a - 0 to 9
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

le       byte 0, 0 ;count of payload bytes
crsrbuf  byte 0, 0, 0
loadadr  byte 0, 0 ;hold start address of loaded prg

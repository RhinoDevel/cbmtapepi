
chrget = $70
chrgot = $76
txtptr = $77 ; two bytes.
mains = $c399+2 ; val. on stack for jsr $0070 from basic main routine (for rts).
d_mode = $0200 ; txtptr will hold this value at chrgot during direct mode call.

; three free bytes:
;
sav_x = chrget+3 ; saved x register content.
sav_y = chrget+4 ; saved y register content.
buf   = chrget+5

cmd_sign = 255 ; command symbol.
emp_sign = $20 ; "empty" character to be used in string.
str_len  = 16

*=634

; install wedge (reuse space later for address, length and command string):
;
addr     lda #$4c ; jmp
len      sta chrget
str      lda #<wedge
         sta chrget+1
         lda #>wedge
         sta chrget+2
         rts
         byte 0
         byte 0
         byte 0
         byte 0
         byte 0
         byte 0
         byte 0

; the wedge:
;
wedge    inc txtptr     ; increment txtptr (because of code overwritten at
         bne savexy     ; chrget with jump to wedge).
         inc txtptr+1   ;

savexy   sty sav_y      ; save original y register content.
         stx sav_x      ; save original x register content.

         tsx            ; check, if call came from basic main routine.
         lda $0101,x    ; check w/o pulling from and pushing (back) to stack.
         cmp #<mains
         bne skip
         lda $0102,x
         cmp #>mains
         bne skip

         lda txtptr     ; check, if call came from basic direct mode.
         cmp #<d_mode
         bne skip
         lda txtptr+1
         cmp #>d_mode
         bne skip

         ldy #0         ;
         lda (txtptr),y ; load current character.
         cmp #cmd_sign  ; check, if this is the command sign.
         bne skip       ;

         ; it is the command character.

         inc txtptr
next_i   lda (txtptr),y
         beq fill_i
         sta str,y
         iny
         cpy #str_len
         bne next_i

         lda (txtptr),y
         bne skip

fill_i   tya
         clc
         adc txtptr     ; txtptr + y
         sta txtptr     ;

         lda #emp_sign
next_f   cpy #str_len
         beq skip
         sta str,y
         iny
         jmp next_f

skip     ldy sav_y      ; restore original y register content.
         ldx sav_x      ; restore original x register content.

         jmp chrgot

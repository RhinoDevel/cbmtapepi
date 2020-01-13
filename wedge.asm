
chrget = $70
chrgot = $76
txtptr = $77 ; two bytes.
mains = $c399+2 ; val. on stack for jsr $0070 from basic main routine (for rts).
d_mode = $0200 ; txtptr will hold this value at chrgot during direct mode call.

; three free bytes:
;
str_i = chrget+3 ; command string character index.
sav_x = chrget+4 ; saved x register content.
sav_y = chrget+5 ; saved y register content.

wait_i   = 255 ; index value to indicate waiting for initial command symbol.
cmd_sign = $21 ; command symbol.
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
         lda #wait_i
         sta str_i
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

         ldy #0         ; for later character loading into a.

         lda str_i      ; check, if waiting for command sign or already reading
         cmp #wait_i    ; entered characters after command sign.
         bne next_i     ;

         ; waiting for command sign.

         lda (txtptr),y ; load current character.
         cmp #cmd_sign  ; check, if this is the command sign.
         beq cmd        ;

         ; it is not the command sign.

skip     ldy sav_y      ; restore original y register content.
         ldx sav_x      ; restore original x register content.
         
         jmp chrgot

cmd      inc str_i      ; set string index from 255 to 0.

done     ldy sav_y      ; restore original y register content.
         ldx sav_x      ; restore original x register content.
         
         jmp chrget

         ; process current character after command sign.

next_i   lda (txtptr),y ; load current character.

         bne fill_i

         ; null => exit with error

         ldx #wait_i
         stx str_i
         jmp skip

         ; fill string at current index with current character:

fill_i   sta str        ; TODO: Implement correctly!

         inc str_i
         lda str_i
         cmp #str_len
         bne done       ; process next character.

         ; string is filled.

         lda #wait_i
         sta str_i
         jmp done

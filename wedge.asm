
chrget   = $70
chrgot   = $76
txtptr   = $77          ; two bytes.
buf      = $0200        ; basic input buffer's address.
cas_buf1 = $027a

cmd_char = 255          ; command symbol.
spc_char = $20          ; "empty" character to be used in string.
str_len  = 16           ; size of command string stored at label "str".

; use the three free bytes behind installed wedge jump:
;
savex    = chrget+3     ; saved x register content.
savey    = chrget+4     ; saved y register content.
;= chrget+5

addr     = cas_buf1
len      = addr+2
str      = len+2

*        = addr

; wedge installer (space reused later for address, length and command string):

         lda #$4c ; jmp
         sta chrget
         lda #<wedge
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

wedge    inc txtptr     ; increment here, because of code overwritten at chrget
         bne savexy     ; with jump to wedge.
         inc txtptr+1

savexy   sty savey      ; save original x and y register contents.
         stx savex

         lda txtptr     ; exec. cmd. in basic direct mode, only.
         cmp #<buf      ; original basic functionality of cmd. char. must still
         bne to_basic   ; work (e.g. pi symbol as constant).
         lda txtptr+1
         cmp #>buf
         bne to_basic

         ldy #0         ; check, if current character is the command sign.
         lda (txtptr),y
         cmp #cmd_char
         bne to_basic

         inc txtptr     ; save at most "str_len" count of characters from input
next_i   lda (txtptr),y ; buffer to "str".
         beq fill_i
         sta str,y
         iny
         cpy #str_len
         bne next_i

         lda (txtptr),y ; there must be a terminating zero following,
         bne to_basic   ; go to basic with character right after cmd. sign,
                        ; otherwise.

fill_i   tya            ; increment txtptr by count of read characters
         clc            ; and fill remaining places in "str" array with spaces.
         adc txtptr
         sta txtptr
         lda #spc_char
next_f   cpy #str_len
         beq to_basic
         sta str,y
         iny
         jmp next_f

to_basic ldy savey      ; restore saved register values and let basic handle
         ldx savex      ; character from input buffer txtptr currently points
         jmp chrgot     ; to.

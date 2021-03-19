
; 2021.03.02
;
; marcel timm, rhinodevel

; cbm pet

; ---------
; functions
; ---------

; *****************
; *** the wedge ***
; *****************

; 16 bytes before the label "wedge" will be reused for cmd. string by wedge!

wedge    inc txtptr     ; increment here, because of code overwritten at chrget
         bne save_y     ; with jump to wedge.
         inc txtptr + 1

str = wedge - str_len ; start of cmd. string buffer in use by wedge.

save_y   sty temp0      ; temporarily save original y register contents.

         ldy txtptr + 1 ; exec. cmd. in basic direct mode, only.
         cpy #>buf      ; original basic functionality of cmd. char. must still
         bne skip       ; work (e.g. pi symbol as constant).
         ldy txtptr
if buf <> $0200 ; better would be (the syntax does not work, here): #<buf <> 0
         cpy #<buf
endif
         bne skip

if buf <> $0200 ; better would be (the syntax does not work, here): #<buf <> 0
         ldy #0
endif
         lda (txtptr),y ; check, if current character is the command sign.
         cmp #cmd_char
         bne skip

         inc txtptr     ; save at most "str_len" count of characters from input.
next_i   lda (txtptr),y ; copy from buffer to "str".
         beq fill_i
str2     sta str,y
         iny
         cpy #str_len
         bne next_i

         lda (txtptr),y ; there must be a terminating zero following,
         bne skip       ; go to basic with character right after cmd. sign,
                        ; otherwise.

fill_i   lda #spc_char  ; fill remaining places in "str" array with spaces.
next_f   cpy #str_len
         beq main
str3     sta str,y
         iny
         bne next_f     ; always branches (saves one byte by not using jmp).

skip     ldy temp0      ; restore saved register y value and let basic handle
         jmp chrgot     ; char. from input buffer txtptr currently points to.

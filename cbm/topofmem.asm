
; 2021.03.03
;
; marcel timm, rhinodevel

; cbm pet

; **********************
; *** basic "loader" ***
; **********************

* = sob

         word bas_next
         word 7581
         byte $9e ; sys token
         byte zer_char + dec_addr1
         byte zer_char + dec_addr2
         byte zer_char + dec_addr3
         byte zer_char + dec_addr4
         byte ":"
         byte $8f; rem token
         text " (c) 2021, rhinodevel"
         byte 0
bas_next word 0

; *************************************************
; *** install by copying to destination address ***
; *************************************************

cpy_inst  

          ; source bottom/start of area:
          ;
src_bot = cpy_src
          ;
          lda #<src_bot
          sta move_bot
          lda #>src_bot
          sta move_bot + 1
 
          ; source top/end of area +1:
          ;
src_top = cpy_lim
          ;
          lda #<src_top
          sta move_src
          lda #>src_top
          sta move_src + 1

          ; destination top/end of area +1:
          ;
dst_top = cas_buf1 + cpy_lim - cpy_src
          ;
          lda #<dst_top
          sta move_dst
          lda #>dst_top
          sta move_dst + 1

          jsr memmove

          ; hard-coded destination address:
          ;
          jmp cas_buf1 ; done, jump to wedge installer.

cpy_src ; source address to begin copying to destination address.

Relocate $027a;cas_buf1 ; hard-coded: assembler does not support constant, here.

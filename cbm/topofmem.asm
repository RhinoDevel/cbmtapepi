
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

cpy_inst  lda #<cpy_src
          sta move_bot
          lda #>cpy_src
          sta move_bot + 1

          lda #<cpy_lim
          sta move_src
          lda #>cpy_lim
          sta move_src + 1

          ; hard-coded destination top of area +1:
          ;          
dst_top = cas_buf1 + cpy_lim - cpy_src;cpy_dst + cpy_lim - cpy_src
          ;
          lda #<dst_top
          sta move_dst
          lda #>dst_top
          sta move_dst + 1

          jsr memmove

          ; hard-coded destination address:
          ;
          jmp cas_buf1;cpy_dst    ; done, jump to wedge installer.

cpy_src ; source address to begin copying to destination address.

Relocate $027a;cas_buf1 ; hard-coded: assembler does not support constant, here.

cpy_dst ; destination address to copy from source address to.

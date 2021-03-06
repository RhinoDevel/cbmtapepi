
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

          ; correct address of wedge to jump to from chrget routine:
          ;
tom_wedg_offset = cpy_lim - wedge    ; offset from wedge to byte following the
                                     ; last byte.
          sec
          lda tomptr
          sbc #<tom_wedg_offset
          sta install_low + 1
          lda tomptr + 1
          sbc #>tom_wedg_offset
          sta install_high + 1

          ; TODO: fix str (and more?)!

          ; source bottom/start of area:
          ;
          lda #<cpy_src
          sta move_bot
          lda #>cpy_src
          sta move_bot + 1
 
          ; source top/end of area +1:
          ;
          lda #<cpy_lim
          sta move_src
          lda #>cpy_lim
          sta move_src + 1
          
          ; destination top/end of area +1:
          ;
          lda tomptr
          sta move_dst
          lda tomptr + 1
          sta move_dst + 1

          jsr memmove

          ; calculate destination bottom/start of area
          ; and update top of memory pointer for basic:
          ;
tom_copy_offset = cpy_lim - cpy_src  ; offset from start of program to byte
                                     ; following the last byte.
          sec
          lda tomptr
          sbc #<tom_copy_offset
          sta tomptr
          lda tomptr + 1
          sbc #>tom_copy_offset
          sta tomptr + 1

          ; TODO: replace marker & offset in jsr addresses with dynamically
          ;       calculated addresses of readbyte() and sendbyte().

          jmp (tomptr) ; done, jump to wedge installer.

tom_send_offset = cpy_lim - sendbyte ; offset from sendbyte() to byte following
                                     ; the last byte.
tom_read_offset = cpy_lim - readbyte ; offset from readbyte() to byte following
                                     ; the last byte.

cpy_src ; source address to begin copying to destination address.

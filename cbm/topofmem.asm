
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

; TODO: do this with using less bytes (using loop and markers?)!

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

          ; correct addresses of str:
          ;
tom_str_offset = cpy_lim - str       ; offset from str to byte following the
                                     ; last byte.
          sec
          lda tomptr
          sbc #<tom_str_offset
          sta str1 + 1
          sta strnext + 1
          sta str2 + 1
          sta str3 + 1
          lda tomptr + 1
          sbc #>tom_str_offset
          sta str1 + 2
          sta strnext + 2
          sta str2 + 2
          sta str3 + 2
  
          ; correct sendbyte() address:
          ;
tom_send_offset = cpy_lim - sendbyte ; offset from sendbyte() to byte following
                                     ; the last byte.
          sec
          lda tomptr
          sbc #<tom_send_offset
          sta send1 + 1
          sta send2 + 1
          sta send3 + 1
          sta send4 + 1
          sta send5 + 1
          sta send6 + 1
          lda tomptr + 1
          sbc #>tom_send_offset
          sta send1 + 2
          sta send2 + 2
          sta send3 + 2
          sta send4 + 2
          sta send5 + 2
          sta send6 + 2

          ; correct readbyte() address:
          ;
tom_read_offset = cpy_lim - readbyte ; offset from readbyte() to byte following
                                     ; the last byte.
          sec
          lda tomptr
          sbc #<tom_read_offset
          sta retrieve + 1
          sta read1 + 1
          sta read_lim + 1
          sta read2 + 1
          sta r_next + 1
          lda tomptr + 1
          sbc #>tom_read_offset
          sta retrieve + 2
          sta read1 + 2
          sta read_lim + 2
          sta read2 + 2
          sta r_next + 2

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
tom_copy_offset = str - cpy_src  ; offset from start of cmd. string buffer to
                                 ; byte following the last byte.
          sec
          lda tomptr
          sbc #<tom_copy_offset
          sta tomptr
          lda tomptr + 1
          sbc #>tom_copy_offset
          sta tomptr + 1

          jmp (tomptr) ; done, jump to wedge installer.

cpy_src ; source address to begin copying to destination address.

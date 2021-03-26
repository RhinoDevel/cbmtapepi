
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

; TODO: do this with less bytes (using loop and markers to copy and replace?)!

;cpy_inst  

          ; *** correct some addresses in source code and top-of-memory ptr. ***

          ldx tomptr                 ; store original top-of-memory address for
          ldy tomptr + 1             ; usage (because tom ptr. will be changed).

          ; correct addresses of str and update top-of-memory pointer:
          ;
tom_str_offset = cpy_lim - wedge - str_len ; offset from str to byte following
                                           ; the last byte.
          sec
          txa
          sbc #<tom_str_offset
          sta str1 + 1
          sta strnext + 1
          sta str2 + 1
          sta str3 + 1
          sta tomptr
          tya
          sbc #>tom_str_offset
          sta str1 + 2
          sta strnext + 2
          sta str2 + 2
          sta str3 + 2
          sta tomptr + 1

          ; correct address of wedge to jump to from chrget routine:
          ;
tom_wedg_offset = cpy_lim - wedge    ; offset from wedge to byte following the
                                     ; last byte.
          sec
          txa
          sbc #<tom_wedg_offset
          sta install_low + 1
          tya
          sbc #>tom_wedg_offset
          sta install_high + 1

          ; correct sendbyte() address:
          ;
tom_send_offset = cpy_lim - sendbyte ; offset from sendbyte() to byte following
                                     ; the last byte.
          sec
          txa
          sbc #<tom_send_offset
          sta send1 + 1
          sta send2 + 1
          sta send3 + 1
          sta send4 + 1
          sta send5 + 1
          sta send6 + 1
          tya
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
          txa
          sbc #<tom_read_offset
          sta retrieve + 1
          sta read1 + 1
          sta read_lim + 1
          sta read2 + 1
          sta r_next + 1
          tya
          sbc #>tom_read_offset
          sta retrieve + 2
          sta read1 + 2
          sta read_lim + 2
          sta read2 + 2
          sta r_next + 2

          ; *** copy modified code to top of memory ***

          ; source bottom/start of area:
          ;
          lda #<wedge
          sta move_bot
          lda #>wedge
          sta move_bot + 1
 
          ; source top/end of area +1:
          ;
          lda #<cpy_lim
          sta move_src
          lda #>cpy_lim
          sta move_src + 1
          
          ; destination top/end of area +1:
          ;
          stx move_dst
          sty move_dst + 1

          jsr memmove


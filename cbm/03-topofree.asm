
; 2021.03.26
;
; marcel timm, rhinodevel

; c64, only.

; ********************************************************************
; *** basic "loader" (to support loading via <shift> + <runs/stop>)***
; ********************************************************************

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

topofree  = $d000 ; top of highest free memory on c64 ($c000-$cfff).
cpy_bytes = cpy_lim - installer
cpy_addr  = topofree - cpy_bytes
cpy_src = jmptoinst + 3 ; to avoid unwanted relocation.
cpy_lim_to_use = cpy_src + cpy_bytes

          ; *** copy code to top of free memory ***

          ; source bottom/start of area:
          ;
          lda #<cpy_src
          sta move_bot
          lda #>cpy_src
          sta move_bot + 1
 
          ; source top/end of area +1:
          ;
          lda #<cpy_lim_to_use
          sta move_src
          lda #>cpy_lim_to_use
          sta move_src + 1
          
          ; destination top/end of area +1:
          ;
          lda #<topofree
          sta move_dst
          lda #>topofree
          sta move_dst + 1

          jsr memmove

jmptoinst jmp installer

Relocate $ced2;cpy_addr ; hard-coded!

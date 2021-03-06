
; 2021.03.03
;
; marcel timm, rhinodevel

; cbm pet

; ***********************
; *** wedge installer *** (space reused after installation for cmd. string)
; ***********************

str      lda #$4c       ; jmp
         sta chrget
install_low
         lda #<wedge
         sta chrget + 1
install_high
         lda #>wedge
         sta chrget + 2
         ;jmp new
         ;byte 0

         lda cas_wrt    ; set write line level to low (7 bytes "wasted",
         and #oudmaskn  ; because not used later by fast mode wedge).
         sta cas_wrt    ;

         jmp new ; repairs start-of-variables pointer, etc., these were changed
                 ; by loading into tape buffer(-s).
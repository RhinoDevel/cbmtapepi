
; 2021.03.03
;
; marcel timm, rhinodevel

; cbm pet

; ***********************
; *** wedge installer *** (space reused after installation for cmd. string)
; ***********************

; 16 bytes (from str label on) are reused for cmd. string by wedge, but the
; other bytes can be used for something else (currently wasted..)!
;
str      lda #$4c       ; jmp
         sta chrget
install_low
         lda #<wedge
         sta chrget + 1
install_high
         lda #>wedge
         sta chrget + 2
         
         lda cas_wrt    ; set write line level to low to signal pi.
         and #oudmaskn  ; 
         sta cas_wrt    ;

if tom_install = 1
         rts
else
         jmp new ; repairs start-of-variables pointer, etc., these were changed
                 ; by loading into tape buffer(-s).
endif

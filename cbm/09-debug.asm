
; 2021.03.02
;
; marcel timm, rhinodevel

; cbm pet

; - - - debugging helpers - - - begin
;
;util_mlm = $fd          ; utility, pointer for machine language monitor, etc.
;videoram = $8000
;
;tapbufin = $bb          ; unused, here: tape buf. #1 & #2 indices to next char
;                        ; (2 bytes).
;
;;; how to output a byte as hexadec. number to some address (e.g. the screen):
;;;
;;         ldx #<videoram
;;         stx util_mlm
;;         ldx #>videoram
;;         stx util_mlm + 1
;;         ldx #$ab
;;         jsr wrt_code         
;
;; ***************************************************************************
;; *** write byte in register x as hex. value in petscii/ascii/screen code ***
;; *** to address stored in zero-page. two bytes will be written,          ***
;; *** most-significant (!) nibble's byte first.                           ***
;; ***************************************************************************
;; *** address to write to is stored in util_mlm.                          ***
;; ***************************************************************************
;; *** modifies register a, x and y.                                       ***
;; ***************************************************************************
;
;wrt_code txa
;         lsr
;         lsr
;         lsr
;         lsr
;         jsr get_code
;         ldy #0
;         sta (util_mlm),y
;
;         txa
;         and #$0f
;         jsr get_code
;         iny
;         sta (util_mlm),y
;
;         rts
;
;; **********************************************************************
;; *** convert byte in register a into petscii / ascii / screen code. ***
;; **********************************************************************
;; *** modifies register a.                                           ***
;; **********************************************************************
;; *** e.g.: 0-9 => 48-57, a-f => 65-70, anything else not supported. ***
;; **********************************************************************
;
;get_code sed
;         clc
;         adc #$90
;         adc #$40
;         cld
;         rts
;
; - - - debugging helpers - - - end

;timer1_low = $e844
;timer1_high = $e845
;via_ifr = $e84d ; via's interrupt flag register.
;
;         ; delay:
;         ;
;         ; $ffff =   65535us
;         ;
;         ;    5s = 5000000us
;         ;
;         ;         5000000us / 65535us = ~77
;         ;
;         ldy #77
;delay    lda #$ff
;         sta timer1_low ; (reading would also clear interrupt flag)
;         sta timer1_high ; clears interrupt flag and starts timer.
;timeout  bit via_ifr ; did timer one time out?
;         bvc timeout
;         dey
;         bne delay

cpy_lim

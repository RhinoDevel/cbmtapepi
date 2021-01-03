
; 2021.01.03
;
; marcel timm, rhinodevel

cas_moto = $e813        ; bit 3 (0 = motor on, 1 = motor off).
ordmask  = 8 ; or mask. bit 3 on <=> motor off.
ordmaskn = 1 not ordmask ; and mask. bit 3 off <=> motor on.

*=634

         sei

loop     lda cas_moto   ; disable motor signal (by enabling bit 3).
         ora #ordmask   ;
         sta cas_moto   ;

         ldx #$ff       ; vice says: #20 => 101 cycles.
initmota dex            ; (motor signal takes its time..)   
         bne initmota

         lda cas_moto   ; motor signal pulse.
         and #ordmaskn  ; disable bit => motor signal to high.
         sta cas_moto   ; 

         ldx #$ff       ; vice says: #20 => 101 cycles.
initmotb dex            ; (motor signal takes its time..)   
         bne initmotb

         jmp loop

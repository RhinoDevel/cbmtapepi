
; 2021.03.02
;
; marcel timm, rhinodevel

; cbm pet

if bas_ver = 1

; basic version 1.
;
bas_buf     = $000a
bas_sob     = $0401
bas_sobptr  = $7a
bas_sovptr  = $7c
bas_tomptr  = $86
bas_chrget  = $c2
bas_chrgot  = $c8
bas_txtptr  = $c9
bas_move_dst = $a7
bas_move_src = $a9
bas_move_bot = $ae
bas_termwili = $06
;
bas_memmove = $c2e1
bas_new     = $c553 ; new is at $c551, but this is without leading syntax check.
bas_rstxclr = $c567
;bas_rstx    = $0000 ; TODO: set correctly!
bas_rechain = $c433
bas_ready   = $c38b ; TODO: does not print anything ($c389, too).

endif

if bas_ver = 2

; basic version 2.
;
bas_buf     = $0200
bas_sob     = $0401
bas_sobptr  = $28
bas_sovptr  = $2a
bas_tomptr  = $34
bas_chrget  = $70
bas_chrgot  = $76
bas_txtptr  = $77
bas_move_dst = $55
bas_move_src = $57
bas_move_bot = $5c
bas_termwili = $0f
;
bas_memmove = $c2df
bas_new     = $c55d ; new is at $c55b, but this is without leading syntax check.
bas_rstxclr = $c572
;bas_rstx    = $c5a7
bas_rechain = $c442
bas_ready   = $c389

endif

if bas_ver = 4

; basic version 4.
;
bas_buf     = $0200
bas_sob     = $0401
bas_sobptr  = $28
bas_sovptr  = $2a
bas_tomptr  = $34
bas_chrget  = $70
bas_chrgot  = $76
bas_txtptr  = $77
bas_move_dst = $55
bas_move_src = $57
bas_move_bot = $5c
bas_termwili = $0f
;
bas_memmove = $b357
bas_new     = $b5d4 ; new is at $b5d2, but this is without leading syntax check.
bas_rstxclr = $b5e9
;bas_rstx    = $b622
bas_rechain = $b4b6
bas_ready   = $b3ff

endif

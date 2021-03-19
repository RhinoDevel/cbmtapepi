
; 2021.03.02
;
; marcel timm, rhinodevel

if bas_ver = 1

; pet basic version 1.

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

bas_memmove = $c2e1
bas_new     = $c553 ; new is at $c551, but this is without leading syntax check.
bas_rstxclr = $c567
bas_rechain = $c433
bas_ready   = $c38b ; TODO: does not print anything ($c389, too).

bas_indamask = %00010000 ; bit 4.
bas_ackmask  = %00001000 ; bit 3.

bas_oudmask  = %00001000 ; bit 3 = 1.
bas_oudmaskn = %11110111 ; bit 3 = 0.
bas_ordmask  = %00001000 ; bit 3.
bas_ordmaskn = %11110111
bas_inackmask = %10000000 ; bit 7.

; using tape #1 port for transfer:

bas_cas_sens = $e810        ; bit 4.
;
; pia 1, port a (59408, tested => correct, 5v/1 when no key pressed or
;                unconnected, "0v"/0 when key pressed).

bas_cas_read = $e811        ; bit 7 is high-to-low flag. pia 1, ctrl.reg. a (59409).
bas_cas_read_reset = $e810  ; pia 1, port a (59408).

bas_cas_wrt  = $e840        ; bit 3.
;
; via, port b (59456, tested => correct, 5v for 1, 0v output for 0).

bas_cas_moto = $e813        ; bit 3 (0 = motor on, 1 = motor off).

endif

if bas_ver = 2

; pet basic version 2.

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

bas_memmove = $c2df
bas_new     = $c55d ; new is at $c55b, but this is without leading syntax check.
bas_rstxclr = $c572
bas_rechain = $c442
bas_ready   = $c389

bas_indamask = %00010000 ; bit 4.
bas_ackmask  = %00001000 ; bit 3.

bas_oudmask  = %00001000 ; bit 3 = 1.
bas_oudmaskn = %11110111 ; bit 3 = 0.
bas_ordmask  = %00001000 ; bit 3.
bas_ordmaskn = %11110111
bas_inackmask = %10000000 ; bit 7.

; using tape #1 port for transfer:

bas_cas_sens = $e810        ; bit 4.
;
; pia 1, port a (59408, tested => correct, 5v/1 when no key pressed or
;                unconnected, "0v"/0 when key pressed).

bas_cas_read = $e811        ; bit 7 is high-to-low flag. pia 1, ctrl.reg. a (59409).
bas_cas_read_reset = $e810  ; pia 1, port a (59408).

bas_cas_wrt  = $e840        ; bit 3.
;
; via, port b (59456, tested => correct, 5v for 1, 0v output for 0).

bas_cas_moto = $e813        ; bit 3 (0 = motor on, 1 = motor off).

endif

if bas_ver = 4

; pet basic version 4.

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

bas_memmove = $b357
bas_new     = $b5d4 ; new is at $b5d2, but this is without leading syntax check.
bas_rstxclr = $b5e9
bas_rechain = $b4b6
bas_ready   = $b3ff

bas_indamask = %00010000 ; bit 4.
bas_ackmask  = %00001000 ; bit 3.

bas_oudmask  = %00001000 ; bit 3 = 1.
bas_oudmaskn = %11110111 ; bit 3 = 0.
bas_ordmask  = %00001000 ; bit 3.
bas_ordmaskn = %11110111
bas_inackmask = %10000000 ; bit 7.

; using tape #1 port for transfer:

bas_cas_sens = $e810        ; bit 4.
;
; pia 1, port a (59408, tested => correct, 5v/1 when no key pressed or
;                unconnected, "0v"/0 when key pressed).

bas_cas_read = $e811        ; bit 7 is high-to-low flag. pia 1, ctrl.reg. a (59409).
bas_cas_read_reset = $e810  ; pia 1, port a (59408).

bas_cas_wrt  = $e840        ; bit 3.
;
; via, port b (59456, tested => correct, 5v for 1, 0v output for 0).

bas_cas_moto = $e813        ; bit 3 (0 = motor on, 1 = motor off).

endif

if bas_ver = 20

; vic 20.

bas_buf     = $0200
bas_sob     = $1001
bas_sobptr  = $2b
bas_sovptr  = $2d
bas_tomptr  = $37 ; unexpanded vic 20 value: $1e00 (7680)
bas_chrget  = $73
bas_chrgot  = $79
bas_txtptr  = $7a
bas_move_dst = $58
bas_move_src = $5a
bas_move_bot = $5f

bas_memmove = $c3bf
bas_new     = $c644 ; new is at $c642, but this is without leading syntax check.
                    ; does not seem to work via jmp (use jsr? wrong address?)!
bas_rstxclr = $c659
bas_rechain = $c533
bas_ready   = $c474

bas_indamask = %01000000 ; bit 6.
bas_ackmask  = %00001000 ; bit 3.
bas_oudmask  = %00001000 ; bit 3 = 1.
bas_oudmaskn = %11110111 ; bit 3 = 0.
bas_ordmask  = %00000010 ; bit 1.
bas_ordmaskn = %11111101
bas_inackmask = %00000010 ; bit 1.

bas_cas_sens = $9111 ; bit 6. via 1, port a (37137).

bas_cas_read = $912d ; bit 1 is high-to-low flag. via 2, interrupt flag reg.
                     ; (37165).
bas_cas_read_reset = $9121  ; via 2, port a (37153).

bas_cas_wrt  = $9120 ; bit 3. via 2, port b (37152).

bas_cas_moto = $911c ; bit 1. via 2, peripheral control reg. (0 = motor on,
                     ; 1 = motor off).

endif

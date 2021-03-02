
; 2021.03.02
;
; marcel timm, rhinodevel

; cbm pet

; ---------------
; system pointers
; ---------------

sobptr   = bas_sobptr   ; pointer to start of basic program.
sovptr   = bas_sovptr   ; pointer to start of basic variables.
txtptr   = bas_txtptr   ; two bytes.
buf      = bas_buf      ; basic input buffer's address.

; used to store program and data:

;termwili = $0f          ; term. width & lim. for scanning src. columns
;                        ; (2 unused bytes).

; ----------------
; system functions
; ----------------

chrget   = bas_chrget
chrgot   = bas_chrgot
new      = bas_new
rstxclr  = bas_rstxclr  ; reset txtptr and perform basic clr command.
;rstx    = bas_rstx      ; (just) reset txtptr.
rechain  = bas_rechain  ; rechain basic program in memory.
ready    = bas_ready    ; print return, "ready.", return and waits for basic
                        ; line or direct command.
; ----------
; peripheral
; ----------

; using tape #1 port for transfer:

cas_sens = $e810        ; bit 4.
;
; pia 1, port a (59408, tested => correct, 5v/1 when no key pressed or
;                unconnected, "0v"/0 when key pressed).

cas_read = $e811        ; bit 7 is high-to-low flag. pia 1, ctrl.reg. a (59409).

cas_wrt  = $e840        ; bit 3.
;
; via, port b (59456, tested => correct, 5v for 1, 0v output for 0).

cas_moto = $e813        ; bit 3 (0 = motor on, 1 = motor off).

; -----------
; "constants"
; -----------

str_len  = 16           ; size of command string stored at label "str".

cmd_char = $21;$ff      ; command symbol. $21 = "!".
sav_char = "+"          ; to save a file (e.g. like "!+myfile.prg").
spc_char = $20          ; "empty" character to be used in string.

; retrieve bytes:
;
indamask = %00010000 ; in-data mask for cas_sens, bit 4.
ackmask  = %00001000 ; out-data-ack. mask for cas_wrt, bit 3.

; send bytes:
;
oudmask  = %00001000 ; out-data mask for cas_wrt, bit 3 = 1 <=> send high.
oudmaskn = %11110111 ; inverted out-data mask for cas_wrt,
                     ; bit 3 = 0 <=> send low.
ordmask  = %00001000 ; out-data-ready mask for cas_moto, bit 3.
                     ; bit 3 = 1 <=> motor off, bit 3 = 0 <=> motor on.

; -----------
; "variables"
; -----------

; use the three free bytes behind installed wedge jump:
;
temp0    = chrget + 3
addr     = chrget + 4 ; 2 bytes.

;lim      = termwili

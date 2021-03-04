
; 2021.03.02
;
; marcel timm, rhinodevel

; cbm pet

; ----------------
; system addresses
; ----------------

sobptr   = bas_sobptr   ; pointer to start of basic program.
sovptr   = bas_sovptr   ; pointer to start of basic variables.
txtptr   = bas_txtptr   ; two bytes.
buf      = bas_buf      ; basic input buffer's address.
sob      = bas_sob      ; start of basic program address.
move_dst = bas_move_dst ; pointer to top of area to be moved to +1.
move_src = bas_move_src ; pointer to top of area to be moved +1.
move_bot = bas_move_bot ; pointer to bottom of area to be moved.
cas_buf1 = $027a        ; cassette buffer 1 and 2 start here (384 bytes).

; used to store program and data:

;termwili = $0f          ; term. width & lim. for scanning src. columns
;                        ; (2 unused bytes).

; ----------------
; system functions
; ----------------

memmove  = bas_memmove  ; move (copy?) memory.
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

; for the basic loader to install at top of memory:
;
dec_addr = 1060;cpy_inst
dec_addr1 = 1;dec_addr / 1000
dec_addr2 = 0;(dec_addr / 100) MOD 10
dec_addr3 = 6;(dec_addr / 10) MOD 10
dec_addr4 = 0;dec_addr MOD 10

str_len  = 16           ; size of command string stored at label "str".

cmd_char = $21;$ff      ; command symbol. $21 = "!".
sav_char = "+"          ; to save a file (e.g. like "!+myfile.prg").
spc_char = $20          ; "empty" character to be used in string.
zer_char = $30          ; zero character for basic loader. $30 = "0".

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

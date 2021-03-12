
; 2021.03.02
;
; marcel timm, rhinodevel

; ----------------
; system addresses
; ----------------

sobptr   = bas_sobptr   ; pointer to start of basic program.
sovptr   = bas_sovptr   ; pointer to start of basic variables.
tomptr   = bas_tomptr   ; pointer to top of memory (e.g. $8000).
txtptr   = bas_txtptr   ; two bytes.
buf      = bas_buf      ; basic input buffer's address.
sob      = bas_sob      ; start of basic program address.
move_dst = bas_move_dst ; pointer to top of area to be moved to +1.
move_src = bas_move_src ; pointer to top of area to be moved +1.
move_bot = bas_move_bot ; pointer to bottom of area to be moved.

; ----------------
; system functions
; ----------------

memmove  = bas_memmove  ; move (copy?) memory.
chrget   = bas_chrget
chrgot   = bas_chrgot
new      = bas_new
rstxclr  = bas_rstxclr  ; reset txtptr and perform basic clr command.
rechain  = bas_rechain  ; rechain basic program in memory.
ready    = bas_ready    ; print return, "ready.", return and waits for basic
                        ; line or direct command.
; ----------
; peripheral
; ----------

cas_sens = bas_cas_sens
cas_read = bas_cas_read
cas_wrt  = bas_cas_wrt
cas_moto = bas_cas_moto

; -----------
; "constants"
; -----------

; start of basic locations:
;
sob_pet = 1025
sob_vic = 4097

; for the basic loader to install at top of memory:
;
if sob = sob_pet ; pet
;dec_addr = 1060;cpy_inst
dec_addr1 = 1;dec_addr / 1000
dec_addr2 = 0;(dec_addr / 100) MOD 10
dec_addr3 = 6;(dec_addr / 10) MOD 10
dec_addr4 = 0;dec_addr MOD 10
endif
if sob = sob_vic ; vic 20
;dec_addr = 4132;cpy_inst
dec_addr1 = 4;dec_addr / 1000
dec_addr2 = 1;(dec_addr / 100) MOD 10
dec_addr3 = 3;(dec_addr / 10) MOD 10
dec_addr4 = 2;dec_addr MOD 10
endif

str_len  = 16           ; size of command string stored at label "str".

cmd_char = $21          ; command symbol. $21 = "!".
sav_char = "+"          ; to save a file (e.g. like "!+myfile.prg").
spc_char = $20          ; "empty" character to be used in string.
zer_char = $30          ; zero character for basic loader. $30 = "0".

; retrieve bytes:
;
indamask = bas_indamask ; in-data mask for cas_sens.
ackmask  = bas_ackmask  ; out-data-ack. mask for cas_wrt.

; send bytes:
;
oudmask  = bas_oudmask  ; out-data mask for cas_wrt.
oudmaskn = bas_oudmaskn ; inverted out-data mask for cas_wrt.
ordmask  = bas_ordmask  ; out-data-ready mask for cas_moto.
                        ; 1 <=> motor off, 0 <=> motor on.

; -----------
; "variables"
; -----------

; use the three free bytes behind installed wedge jump:
;
temp0    = chrget + 3
addr     = chrget + 4 ; 2 bytes.

;lim      = <add address of some "unused" 2 byte long place in zero-page, here>

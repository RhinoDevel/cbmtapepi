
; 2021.01.03
;
; marcel timm, rhinodevel

; cbm pet

bas_ver = 4; pet basic version to assemble for. can be set to 1, 2 or 4.

if bas_ver = 1

; basic version 1.
;
bas_rstxclr = $0000 ; TODO: set correctly!
;bas_rstx    = $0000 ; TODO: set correctly!
bas_rechain = $0000 ; TODO: set correctly!
bas_ready    = $0000 ; TODO: set correctly!
;
; TODO: add other stuff that is different!

else
if bas_ver = 2

; basic version 2.
;
bas_new = $c55d ; new is at $c55b, but this is without leading syntax check.
bas_rstxclr = $c572
;bas_rstx    = $c5a7
bas_rechain = $c442
bas_ready    = $c389

else

; assuming basic version 4.
;
bas_new = $b5d4 ; new is at $b5d2, but this is without leading syntax check.
bas_rstxclr = $b5e9
;bas_rstx    = $b622
bas_rechain = $b4b6
bas_ready    = $b3ff

endif
endif

; ---------------
; system pointers
; ---------------

basstptr = $28          ; pointer to start of basic program.
varstptr = $2a          ; pointer to start of basic variables.
txtptr   = $77          ; two bytes.
buf      = $0200        ; basic input buffer's address.

; used to store program and data:

;termwili = $0f          ; term. width & lim. for scanning src. columns
;                        ; (2 unused bytes).

; ----------------
; system functions
; ----------------

chrget   = $70
chrgot   = $76
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

; ---------
; functions
; ---------

; 192 bytes long tape buffer is filled with:
;
; 1)   1 byte  = $027a - $027a, tape filetype (probably).
; 2)   2 bytes = $027b - $027c, prg start address.
; 3)   2 bytes = $027d - $027e, address after last byte of prg [(3) - (2) = prg byte count].
; 4)  16 bytes = $027f - $028e, file name (petscii, padded with blanks/$20).
; 5) 171 bytes = $028f - $0339, additional bytes.
;
; file name = "fastmode: sys655" => 
;    171 bytes = $028f - $0339, source code in tape buffer #1.
;    192 bytes = $033a - $03f9, source code in tape buf. #2 (192 bytes avail.).
;
;    =>
;    363 bytes available.
;
;                (over-)write the following for basic loader usage:
;
;                $03fa - $03ff: $a4 $d7 (v2: $f7 $e7), mlm usrcmd ext. vector.
;                               $00                  , ieee "timeout defeat".
;                               $ff [v4 40 col. real machine: $f7] ; unused.
;                               $ff [v4 40 col. real machine: $f7] ; unused.
;                               $ff [v4 40 col. real machine: $d8] ; unused.
;                $0400 - $0400, $00                  , 0-byte before basic prg.
;
;                $0401,         start of basic.

* = $028f ; (see explanation above)

; ***********************
; *** wedge installer *** (space partially reused after install for cmd. string)
; ***********************

str      lda #$4c       ; jmp
         sta chrget
         lda #<wedge
         sta chrget + 1
         lda #>wedge
         sta chrget + 2
         ;jmp new
         ;byte 0

         lda cas_wrt    ; set write line level to low (7 bytes "wasted",
         and #oudmaskn  ; because not used later by fast mode wedge).
         sta cas_wrt    ;

         jmp new ; repairs start-of-variables pointer, etc., these were changed
                 ; by loading into tape buffer(-s).

; *****************
; *** the wedge ***
; *****************

wedge    inc txtptr     ; increment here, because of code overwritten at chrget
         bne save_y     ; with jump to wedge.
         inc txtptr + 1

save_y   sty temp0      ; temporarily save original y register contents.

         ldy txtptr + 1 ; exec. cmd. in basic direct mode, only.
         cpy #>buf      ; original basic functionality of cmd. char. must still
         bne skip       ; work (e.g. pi symbol as constant).
         ldy txtptr
         ;cpy #<buf     ; hard-coded: commented-out for "buf" = $??00, only!
         bne skip

         ;ldy #0        ; hard-coded: commented-out for "buf" = $??00, only!
         lda (txtptr),y ; check, if current character is the command sign.
         cmp #cmd_char
         bne skip

         inc txtptr     ; save at most "str_len" count of characters from input.
next_i   lda (txtptr),y ; copy from buffer to "str".
         beq fill_i
         sta str,y
         iny
         cpy #str_len
         bne next_i

         lda (txtptr),y ; there must be a terminating zero following,
         bne skip       ; go to basic with character right after cmd. sign,
                        ; otherwise.

fill_i   lda #spc_char  ; fill remaining places in "str" array with spaces.
next_f   cpy #str_len
         beq main
         sta str,y
         iny
         bne next_f     ; always branches (saves one byte by not using jmp).

skip     ldy temp0      ; restore saved register y value and let basic handle
         jmp chrgot     ; char. from input buffer txtptr currently points to.

; ************
; *** main ***
; ************

         ; (byte at temp0 can be reused from here on)

main     sei

         ; commented-out, because directly using varstptr during send and
         ; retrieve:
         ;
         ;lda varstptr
         ;sta lim
         ;lda varstptr + 1
         ;sta lim + 1

         lda #0
         sta addr
         sta addr + 1
         
         tax ; (for sending command string, below)

         lda str
         cmp #sav_char
         bne addrlim_rdy

         lda basstptr
         sta addr
         lda basstptr + 1
         sta addr + 1
addrlim_rdy

; >>> send bytes: <<<

         ; motor signal must already be low or on its way to low, here.      

         bit cas_read-1 ; maybe not needed: makes sure that flag raised by
                        ;                   high-to-low on read line is not
                        ;                   raised (see pia documentation).

         ;ldx #0        ; send command string.
strnext  ldy str,x
         jsr sendbyte
         inx
         cpx #str_len
         bne strnext

         ldy addr       ; send address.
         jsr sendbyte
         ldy addr + 1
         jsr sendbyte

         lda addr       ; address is zero => no limit and payload to send.
         bne send_lim
         lda addr + 1
         beq retrieve

send_lim ldy varstptr;lim         ; send first address above payload ("limit").
         jsr sendbyte
         ldy varstptr + 1;lim + 1
         jsr sendbyte

s_next   ;ldy #0        ; (y is always 0 after sendbyte) ; send payload.
         lda (addr),y
         tay
         jsr sendbyte
         inc addr       ; increment to next (read) address.
         bne s_finchk
         inc addr + 1
s_finchk lda addr       ; check, if end is reached.
         cmp varstptr;lim
         bne s_next
         lda addr + 1
         cmp varstptr + 1;lim + 1
         bne s_next

; >>> retrieve bytes: <<<

; expected values at this point:
;
; cas_sens = data input.
; cas_read = data-ready input, don't care about level, just care about
;            high -> low change.
;
; cas_wrt  = data-ack. output. current level was decided by sending done, above.

retrieve jsr readbyte  ; read address.
         sta addr
         jsr readbyte
         sta addr + 1
         
         ; [cmp #0 is not necessary, because of last lda temp0 in readbyte()]
         ;
         ;lda addr + 1
         ;cmp #0        ; exit, if addr. is 0.
         bne read_lim
         lda addr
         beq exit      ; todo: overdone and maybe unwanted (see label)!

read_lim jsr readbyte  ; read payload "limit" (first addr. above payload).
         sta varstptr;lim
         jsr readbyte
         sta varstptr + 1;lim + 1

r_next   jsr readbyte   ; retrieve payload.
         ;ldx #0        ; [x is always 0 after readbyte()]
         sta (addr,x)   ; store byte at current address.
         inc addr       ; increment to next (write) address.
         bne r_finchk
         inc addr + 1
r_finchk lda addr                 ; check, if end is reached.
         cmp varstptr;lim
         bne r_next
         lda addr + 1
         cmp varstptr + 1;lim + 1
         bne r_next

         ;;lda addr + 1
         ;sta varstptr + 1 ; set basic variables start pointer to behind loaded
         ;lda addr         ; payload.
         ;sta varstptr     ;

exit     cli

         jsr rstxclr    ; equals preparations after basic load at $b4ad/$c439.
         jsr rechain    ;

         jmp ready

; ********************************************************
; *** send a byte from register y.                     ***
; ********************************************************
; *** modifies registers a, y and memory at temp0.     ***
; ********************************************************

sendbyte sty temp0      ; byte buffer during send.
         ldy #8         ; (send bit) counter.

sendloop lsr temp0      ; sends current bit to c flag.

         lda cas_wrt    ; (does not change c flag)
         and #oudmaskn  ; set bit to zero to send 0/low (does not change c fl.).
         bcc senddata
         ora #oudmask   ; set bit to one to send 1/high.
senddata sta cas_wrt    ; set data bit.

         lda cas_moto   ; motor signal toggle.
         eor #ordmask   ;
         sta cas_moto   ;

sendwait bit cas_read   ; wait for data-ack. high-low (writes bit 7 to n flag).
         bpl sendwait   ; branch, if n is 0 ("positive").

         bit cas_read-1 ; resets "toggle" bit by read operation (see pia doc.).

         dey
         bne sendloop   ; last bit read?

         rts            ; y is 0, here!

; ********************************************************
; *** read a byte into register a and memory at temp0. ***
; ********************************************************
; *** modifies registers a, x and memory at temp0.     ***
; ********************************************************

readbyte ldx #8         ; (read bit) counter.

readwait bit cas_read   ; wait for data-ready toggling (writes bit 7 to n flag).
         bpl readwait   ; branch, if n is 0 ("positive").

         bit cas_read-1 ; resets "toggle" bit by read operation (see pia doc.).

         lda cas_sens   ; load actual data (bit 4) into c flag.
         and #indamask  ; sets z flag to 1, if bit 4 is 0.
         clc            ;
         beq readadd    ; bit read is zero.
         sec            ;

readadd  ror temp0      ; put read bit from c flag into byte buffer.

         lda cas_wrt    ; acknowledge data bit ("toggle" data-req. line level).
         eor #ackmask   ; toggle output bit.
         sta cas_wrt    ;

         dex
         bne readwait   ; last bit read?

         lda temp0
         rts            ; read byte is in register a and in memory at temp0,
                        ; x is 0, here!

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

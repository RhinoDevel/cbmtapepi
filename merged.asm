
; 2020.01.15
;
; marcel timm, rhinodevel

; cbm pet

; configured for basic v4.
;
; if an address in basic v2 is different, the basic v2 address is included as
; commented-out value directly after the basic v4 value.

; ---------------
; system pointers
; ---------------

varstptr = $2a          ; pointer to start of basic variables.
txtptr   = $77          ; two bytes.
buf      = $0200        ; basic input buffer's address.

; used to store program and data:

termwili = $0f          ; term. width & lim. for scanning src. columns
                        ; (2 unused bytes).
tapbufin = $bb          ; tape buf. #1 & #2 indices to next char (2 bytes).
cas_buf1 = $027a        ; cassette buffer 1 and 2 start here (384 bytes).

; ----------------
; system functions
; ----------------

chrget   = $70
chrgot   = $76
rstxclr  = $b5e9;$c572  ; reset txtptr and perform basic clr command.
;rstx    = $b622;$c5a7   ; (just) reset txtptr.
rechain  = $b4b6;$c442  ; rechain basic program in memory.
ready    = $b3ff;$c389  ; print return, "ready.", return and waits for basic
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

moto_del = 20           ; to wait approximately 100 microseconds (see usage).
str_len  = 16           ; size of command string stored at label "str".

cmd_char = $21;$ff      ; command symbol. $21 = "!".
spc_char = $20          ; "empty" character to be used in string.

; retrieve bytes:
;
out_req  = cas_wrt
in_ready = cas_read
in_data  = cas_sens
reqmask  = %00001000 ; out-data-request mask. bit 3.
indamask = %00010000 ; in-data mask. bit 4.

; send bytes:
;
out_rdy  = cas_moto
in_req   = cas_sens
outdata  = cas_wrt
ordmask  = %00001000 ; out-ready or-mask. bit 3 on <=> motor off.
ordmaskn = %11110111 ; out-ready and-mask. bit 3 off <=> motor on.
oudmask  = %00001000 ; out-data mask. bit 3.
oudmaskn = %11110111 ; inverted out-data mask.
ireqmask = %00010000 ; in-data-request mask. bit 4.

; -----------
; "variables"
; -----------

; use the three free bytes behind installed wedge jump:
;
temp0    = chrget + 3
temp1    = chrget + 4
temp2    = chrget + 5

addr     = termwili
lim      = tapbufin

str      = cas_buf1

*        = str

; ---------
; functions
; ---------

; ***********************
; *** wedge installer *** (space reused after installation for cmd. string)
; ***********************

         lda #$4c       ; jmp
         sta chrget
         lda #<wedge
         sta chrget + 1
         lda #>wedge
         sta chrget + 2
         rts
         byte 0
         byte 0
         byte 0

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

; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
; TODO: handle "addr" and "lim" setup (0 for commands, filled for saving)!
; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
; TODO (probably on pi): handle pi error (e.g. file or sub folder not found)!
; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
; TODO: sometimes load does not work (invader.prg / 7169 bytes???, tetris.prg)!
;
;       next steps: increase motor waits,
;                   add debug output on pet,
;                   log more on pi.
; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
         ;
         lda #0
         sta addr
         sta addr + 1
         sta lim
         sta lim + 1

; >>> send bytes: <<<

         ;lda #0
         sta temp0      ; make sure to initially wait for data-req. low.

         lda out_rdy    ; disable motor signal (by enabling bit 3).
         ora #ordmask   ; motor signal should already be disabled, but anyway..
         sta out_rdy    ;

         ldx #moto_del  ; vice says: 20 <=> 101 cycles.
initmotd dex            ; (motor signal takes its time..)
         bne initmotd

         ldx #0         ; send command string.
strnext  ldy str,x
         stx temp1
         jsr sendbyte
         ldx temp1
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

send_lim ldy lim        ; send first address above payload ("limit").
         jsr sendbyte
         ldy lim + 1
         jsr sendbyte

s_next   ldy #0         ; send payload.
         lda (addr),y
         tay
         jsr sendbyte
         inc addr       ; increment to next (read) address.
         bne s_finchk
         inc addr + 1
s_finchk lda addr       ; check, if end is reached.
         cmp lim
         bne s_next
         lda addr + 1
         cmp lim + 1
         bne s_next

; >>> retrieve bytes: <<<

; wait for initial request from pi to send data to commodore:
;
retrieve lda in_req     ; wait for retrieve data-req. line to go low.
         and #ireqmask  ; => register a holds either 00010000 or 00000000.
         bne retrieve   ; (default value of sense line is high)

; expected values at this point:
;
; cas_wrt/out_req   = output, current level was decided by sending done, above.
; cas_read/in_ready = input, don't care about level, just care about high -> low
;                     change.
; cas_sens/in_data  = input.

         jsr readbyte  ; read address.
         sta addr
         jsr readbyte
         sta addr + 1

         ;lda addr + 1
         cmp #0       ; exit, if addr. is 0.
         bne read_lim
         lda addr
         beq exit      ; todo: overdone and maybe unwanted (see label)!

read_lim jsr readbyte  ; read payload "limit" (first addr. above payload).
         sta lim
         jsr readbyte
         sta lim + 1

r_next   jsr readbyte   ; retrieve payload.
         ldy #0
         sta (addr),y   ; store byte at current address.
         inc addr       ; increment to next (write) address.
         bne r_finchk
         inc addr + 1
r_finchk lda addr       ; check, if end is reached.
         cmp lim
         bne r_next
         lda addr + 1
         cmp lim + 1
         bne r_next

         ;lda addr + 1
         sta varstptr + 1 ; set basic variables start pointer to behind loaded
         lda addr         ; payload.
         sta varstptr     ;

exit     cli

         jsr rstxclr    ; equals preparations after basic load at $c439.
         jsr rechain    ;

         jmp ready

; **************************************
; *** send a byte from register y.   ***
; **************************************
; *** modifies registers a, x and y. ***
; ***                                ***
; *** modifies temp0.                ***
; **************************************

sendbyte ldx #8         ; (send bit) counter.

sendloop lda in_req     ; wait for data-req. line to toggle.
         and #ireqmask  ; => register a holds either 00010000 or 00000000.

         cmp temp0
         bne sendloop

         eor #ireqmask  ; toggle expected next data-req. value
         sta temp0      ; between 00000000 and 00010000 (low or high).

         tya
         lsr            ; sends current bit to c flag.
         tay            ; (does not change c flag)

         lda outdata    ; (does not change c flag)
         and #oudmaskn  ; set bit to zero to send 0/low (does not change c fl.).
         bcc senddata
         ora #oudmask   ; set bit to one to send 1/high.

senddata sta outdata    ; set data bit.

         lda out_rdy    ; motor signal pulse.
         and #ordmaskn  ; disable bit => motor signal to high.
         sta out_rdy    ;

         txa            ;
         ldx #moto_del  ; vice says: 20 <=> 101 cycles.
sendmotd dex            ; (motor signal takes its time and its a pulse..)
         bne sendmotd   ;
         tax            ;

         lda out_rdy    ;
         eor #ordmask   ; enable bit => motor signal back to low.
         sta out_rdy    ;

         dex
         bne sendloop   ; last bit read?

         rts

; **************************************
; *** read a byte into register a.   ***
; **************************************
; *** modifies registers a, x and y. ***
; **************************************

readbyte lda #0         ; byte buffer during read (also y).
         ldx #8         ; (read bit) counter.

readloop tay
         lda out_req    ; req. next data bit ("toggle" data-request line level).
         eor #reqmask   ; toggle output bit.
         sta out_req    ;

readwait bit in_ready   ; wait for data-ready toggling (writes bit 7 to n flag).
         bpl readwait   ; branch, if n is 0 ("positive").

         bit in_ready-1 ; resets "toggle" bit by read operation (see pia doc.).

         lda in_data    ; load actual data (bit 4) into c flag.
         and #indamask  ; sets z flag to 1, if bit 4 is 0.
         clc            ;
         beq readadd    ; bit read is zero.
         sec            ;

readadd  tya
         ror            ; put read bit from c flag into byte buffer.

         dex
         bne readloop   ; last bit read?

         rts            ; read byte is in register a (and y).

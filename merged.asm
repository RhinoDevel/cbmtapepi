
; 2020.01.15
;
; marcel timm, rhinodevel

; cbm pet

; configured for basic 2.0 / rom v3.

; ---------------
; system pointers
; ---------------

varstptr = 42;124      ; pointer to start of basic variables.
txtptr   = $77          ; two bytes.
buf      = $0200        ; basic input buffer's address.
cas_buf1 = $027a

; ----------------
; system functions
; ----------------

chrget   = $70
chrgot   = $76
clr      = $c577;$c56a or $c770?
rechain  = $c442;$c433 or $c430?

; -----------
; "constants"
; -----------

counter  = $e849    ; read timer 2 counter high byte.
del      = 1        ; (see function for details) ; todo: 100us would be enough.

tapbufin = $bb;$271 ; tape buf. #1 & #2 indices to next char (2 bytes).
adptr    = 15;6 ; term. width & lim. for scanning src. columns (2 unused bytes).

; using tape #1 port for transfer:

cas_sense = $e810 ; bit 4.
;
; pia 1, port a (59408, tested => correct, 5v/1 when no key pressed or
;                unconnected, "0v"/0 when key pressed).

cas_read = $e811 ; bit 7 is high-to-low flag. pia 1, control register a (59409).

cas_write = $e840 ; bit 3
;
; via, port b (59456, tested => correct, 5v for 1, 0v output for 0).

cas_motor = $e813 ; bit 3 (0 = motor on, 1 = motor off).

; retrieve bytes:
;
out_req  = cas_write
in_ready = cas_read
in_data  = cas_sense
reqmask  = 8 ; bit 3.
reqmaskn = 1 not reqmask ; (val. before not operator does not seem to matter)
indamask = $10 ; bit 4.

; send bytes:
;
out_rdy  = cas_motor
in_req   = cas_sense
outdata  = cas_write
ordmask  = 8 ; or mask. bit 3 on <=> motor off.
ordmaskn = 1 not ordmask ; and mask. bit 3 off <=> motor on.
oudmask  = 8 ; bit 3.
oudmaskn = 1 not oudmask ; (val. before not operator does not seem to matter)
ireqmask = $10 ; bit 4.

cmd_char = 255          ; command symbol.
spc_char = $20          ; "empty" character to be used in string.
str_len  = 16           ; size of command string stored at label "str".

; use the three free bytes behind installed wedge jump:
;
savex    = chrget+3     ; saved x register content.
savey    = chrget+4     ; saved y register content.
temp     = chrget+5

; ---------
; variables
; ---------

addr     = cas_buf1
len      = addr+2
str      = len+2

*        = addr

; ---------
; functions
; ---------

; ***********************
; *** wedge installer *** (space reused later for addr., len. and cmd. string)
; ***********************

         lda #$4c ; jmp
         sta chrget
         lda #<wedge
         sta chrget+1
         lda #>wedge
         sta chrget+2
         rts
         byte 0
         byte 0
         byte 0
         byte 0
         byte 0
         byte 0
         byte 0

; *****************
; *** the wedge ***
; *****************

wedge    inc txtptr     ; increment here, because of code overwritten at chrget
         bne savexy     ; with jump to wedge.
         inc txtptr+1

savexy   sty savey      ; save original x and y register contents.
         stx savex

         ldy txtptr+1   ; exec. cmd. in basic direct mode, only.
         cpy #>buf      ; original basic functionality of cmd. char. must still
         bne to_basic   ; work (e.g. pi symbol as constant).
         ldy txtptr
         ;cpy #<buf     ; hard-coded: commented-out for "buf" = $??00, only!
         bne to_basic

         ; hard-coded for y already holding 0 (see above)!
         ;
         ;ldy #0         ; check, if current character is the command sign.
         lda (txtptr),y
         cmp #cmd_char
         bne to_basic

         ;ldy #0
         inc txtptr     ; save at most "str_len" count of characters from input
next_i   lda (txtptr),y ; buffer to "str".
         beq fill_i
         sta str,y
         iny
         cpy #str_len
         bne next_i

; TODO: Include this again!
;
;         lda (txtptr),y ; there must be a terminating zero following,
;         bne to_basic   ; go to basic with character right after cmd. sign,
;                        ; otherwise.

fill_i   tya            ; increment txtptr by count of read characters
         clc            ; and fill remaining places in "str" array with spaces.
         adc txtptr
         sta txtptr
         lda #spc_char
next_f   cpy #str_len
         beq main;to_main
         sta str,y
         iny
         jmp next_f

;to_main  jsr main

to_basic ldy savey      ; restore saved register values and let basic handle
         ldx savex      ; character from input buffer txtptr currently points
         jmp chrgot     ; to.

; ************
; *** main ***
; ************

; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
; TODO: Handle "addr" and "len" setup (0 for commands, filled for saving)!
; * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

main     sei

; >>> send bytes: <<<

         lda #0         ; make sure to initially wait for data-req. low.
         sta sendtog+1  ;

; TODO: OK?
;
;         lda out_rdy    ; disable motor signal (by enabling bit 3).
;         ora #ordmask   ; motor signal should already be disabled, but anyway..
;         sta out_rdy    ;
;         jsr waitdel    ; (motor signal takes its time..)
;
         tax
         ;
         ;ldx #0         ; send string.

strnext  ldy str,x
         stx temp
         jsr sendbyte
         ldx temp
         inx
         cpx #str_len
         bne strnext

         ldy addr       ; send address (and fill address pointer for later).
         sty adptr
         jsr sendbyte
         ldy addr+1
         sty adptr+1
         jsr sendbyte

         ldy len        ; send length.
         jsr sendbyte
         ldy len+1
         jsr sendbyte

         lda len
         bne send_pl
         lda len+1
         beq retrieve   ; length is zero. => no payload to send.

send_pl  clc            ; put first address above data into buffer.
         lda adptr
         adc len
         sta tapbufin
         lda adptr+1
         adc len+1
         sta tapbufin

pl_next  ldy #0         ; send payload.
         lda (adptr),y
         tay
         jsr sendbyte

         inc adptr      ; increment to next (read) address.
         bne plfinchk
         inc adptr+1

plfinchk lda adptr      ; check, if end is reached.
         cmp tapbufin
         bne pl_next
         lda adptr+1
         cmp tapbufin+1
         bne pl_next

; >>> retrieve bytes: <<<

; wait for initial request to send data from pi to commodore:
;

retrieve lda in_req     ; wait for retrieve data-req. line to go low.
         and #ireqmask  ; => register a holds either 00010000 or 00000000.
         bne retrieve   ; (default value of sense line is high)

; expected values at this point:
;
; cas_write/out_req = output, current level was decided by sending done, above.
; cas_read/in_ready = input, don't care about level, just care about high -> low
;                     change.
; cas_sense/in_data = input.

         jsr readbyte  ; read payload byte count.
         sty tapbufin
         jsr readbyte
         sty tapbufin+1

         cpy #0        ; exit, if byte count is zero.
         bne readaddr
         lda tapbufin
         beq exit

readaddr jsr readbyte  ; read start address.
         sty adptr
         jsr readbyte
         sty adptr+1

nextpl   lda tapbufin
         bne declelo
         lda tapbufin+1 ; low byte is zero.
         beq readdone
         dec tapbufin+1 ; high byte is not zero (but low byte is).
declelo  dec tapbufin

         jsr readbyte  ; read byte.

         tya           ;
         ldy #0        ;
         sta (adptr),y ; store byte at current address.

         inc adptr
         bne nextpl
         inc adptr+1
         jmp nextpl

readdone lda adptr+1    ; set basic variables start pointer to behind loaded
         sta varstptr+1 ; prg.
         lda adptr      ;
         sta varstptr   ;

         jsr clr
         jsr rechain

exit     cli
         jmp to_basic;rts

; **************************************
; *** send a byte from register y    ***
; **************************************
; *** modifies registers a, x and y. ***
; **************************************

sendbyte ldx #8         ; (send bit) counter.

sendloop lda in_req     ; wait for data-req. line to toggle.
         and #ireqmask  ; => register a holds either 00010000 or 00000000.

sendtog  cmp #0         ; (value will be changed in-place)
         bne sendloop

         eor #ireqmask  ; toggle expected next data-req. value
         sta sendtog+1  ; between 00000000 and 00010000 (low or high).

         tya
         lsr            ; sends current bit to c flag.
         tay            ; (does not change c flag)

         lda outdata    ; (does not change c flag)
         bcc sendzer
         ora #oudmask   ; set bit to one to send 1 (high).
         jmp senddata
sendzer  and #oudmaskn  ; set bit to zero to send 0 (low).

senddata sta outdata    ; set data bit.

         lda out_rdy    ; motor signal pulse.
         and #ordmaskn  ; disable bit => motor signal to high.
         sta out_rdy    ;

         ;jsr waitdel    ; (motor signal takes its time and its a pulse..)
         ;
         lda #del
         sta counter
delay    cmp counter
         bcs delay      ; branch, if "del" is equal or greater than counter.

         lda out_rdy    ;
         eor #ordmask   ; enable bit => motor signal back to low.
         sta out_rdy    ;

         dex
         bne sendloop   ; last bit read?

         rts

; **************************************
; *** read a byte into register y    ***
; **************************************
; *** modifies registers a, x and y. ***
; **************************************

readbyte ldy #0         ; byte buffer during read.
         ldx #8         ; (read bit) counter.

readloop lda out_req    ; req. next data bit ("toggle" data-request line level).
         and #reqmask
         beq toghigh
         lda out_req    ; toggle output to low.
         and #reqmaskn
         jmp togdo
toghigh  lda out_req    ; toggle out_req output to high.
         ora #reqmask
togdo    sta out_req    ; [does not work in vice (v3.1)]

readwait bit in_ready   ; wait for data-ready toggling (writes bit 7 to n flag).
         bpl readwait   ; branch, if n is 0 ("positive").

         bit in_ready-1 ; resets "toggle" bit by read operation (see pia doc.).

         lda in_data    ; load actual data (bit 4) into c flag.
         clc            ;
         and #indamask  ; sets z flag to 1, if bit 4 is 0.
         beq readadd    ; bit read is zero.
         sec            ;

readadd  tya            ; put read bit from c flag into byte buffer.
         ror            ;
         tay            ;

         dex
         bne readloop   ; last bit read?

         rts            ; read byte is in register y.

; ************************************************************
; *** wait constant "del" multiplied by 256 microseconds   ***
; ************************************************************
; *** modifies register a.                                 ***
; ************************************************************
;
;waitdel  lda #del
;         sta counter
;delay    cmp counter
;         bcs delay      ; branch, if "del" is equal or greater than counter.
;         rts

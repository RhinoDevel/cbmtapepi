
; 2021.01.03
;
; marcel timm, rhinodevel

; cbm pet

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

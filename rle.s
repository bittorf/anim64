; Copyright (c) 2004, Per Olofsson
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
;
; * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
; * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
; IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
; FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
; CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
; DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
; DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
; IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
; OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;
; Routines for packing and unpacking run length encoded byte streams
;
; When two or more consecutive bytes are identical, they are replaced by
; <BYTE> <BYTE> <COUNT>. A COUNT of 0 indicates End Of Stream. A COUNT
; of 1 indicates that two bytes should be written, a COUNT of 2 indicates
; three bytes, and so on.


  	.export rle_read, rle_store
  	.exportzp src, dest
	.export lastbyte
	.export destlen


	.importzp ptr1, ptr2


	.zeropage

src		= ptr1		; borrow cc65's temp pointers
dest		= ptr2


	.bss

lastbyte:	.res 1		; last byte read
destlen:	.res 2		; number of bytes written


	.code


; read a byte and increment source pointer
rle_read:
	lda (src),y
	inc src
	bne :+
	inc src + 1
:	rts


; write a byte and increment destination pointer
rle_store:
	sta (dest),y
	inc dest
	bne :+
	inc dest + 1
:	inc destlen
	bne :+
	inc destlen + 1
:	rts

	%macro prologue 0
	push rbp
	mov rbp,rsp
	push rbx
	push r12
	push r13
	push r14
	push r15
	%endmacro
	
	%macro epilogue 0
	pop r15
	pop r14
	pop r13
	pop r12
	pop rbx
	mov rsp,rbp
	pop rbp
	%endmacro
	
section .data
	gf28_modulo dw 0x011b

section .bss
	aes_mul_gf28_scratchpad resb 7 ;There will be 7 maximum members of GF2^8 we need to keep.

	;; Store the values for sbox, they will take 512 bytes, nothing by today's standards.
	sub_bytes_arr resb 0x100
	sub_bytes_inv_arr resb 0x100
	
section .text
	global aes_mul_gf28
	global aes_mul_poly
	global aes_inv_gf28
	
;;; aes_mul_poly multiplies two polynomials with coefficients in GF2^8
;;; Preconditions:
;;; 	rdi is 0 everywhere but edx
;;; 	rsi is 0 everywhere but esi
;;; Parameters:
;;; 	edi contains the first polynomial
;;; 	esi contains the second polynomial
;;; Returns:
;;; 	eax contains the result of multiplication, rax is 0 everywhere but eax
aes_mul_poly:
	prologue
	mov r12,rdi		;Preserve args
	mov r13,rsi		;-

	mov rcx,0x4		;matrix multiplication of a 4x4 with a 4x1 uses 0x10 ops
	xor rax,rax
aes_mul_poly_column_next:	;outer loop
	push rax
	push rcx
	mov rcx,0x4
	xor rax,rax		;init rax to 0 so our first result in each coefficient will be good
	
aes_mul_poly_row_next:		;inner loop
	
	mov rbx,rax		;move accumulation to rbx

	;; gf28 mul the numbers
	push rdi
	push rsi
	push rcx
	and rdi,0xff
	and rsi,0xff
	call aes_mul_gf28
	pop rcx
	pop rsi
	pop rdi
	
	xor rax,rbx		;gf28 add accumulation to return value and store in rax

	;; move the masks to the next term
	rol edi,0x8
	ror esi,0x8

	loop aes_mul_poly_row_next ;inner loop
	pop rcx
	shl rax,24		;put this in the msB
	mov r10,rax		;move intermediate result (coefficient) to r10
	pop rax
	shr rax,8
	or rax,r10
	
	;; Don't rotate when we're done with each coefficient.
	ror edi,0x8
	
	loop aes_mul_poly_column_next ;outer loop
	epilogue
	ret

;;; aes_mul_gf28 multiplies two bytes representing the polynomials in GF2^8
;;; Preconditions:
;;; 	rdi is 0 everywhere but dil
;;; 	rsi is 0 everywhere but sil
;;; Postconditions:
;;; 	rax is 0 everywhere but al
;;; Parameters:
;;; 	dil contains the first polynomial
;;; 	sil contains the second polynomial
;;; Returns:
;;; 	rax contains the result of GF2^8 multiplication
aes_mul_gf28:
	prologue
	mov r10,rdi		;Preserve our arguments
	mov r11,rsi		;-

	mov rcx,0x7
aes_mul_gf28_next_xtime:

	push r10
	push r11
	call xtime
	pop r11
	pop r10
	
	mov rdi,rax
	mov [aes_mul_gf28_scratchpad + rcx - 1],al
	loop aes_mul_gf28_next_xtime

	mov rcx,7
	xor rax,rax
	shr r11b,1		;handle the 1s place separately (for efficiency)
	jnc aes_mul_gf28_next_xor	;if we have a 0 in the ones place, we can leave rbx 0
	mov rax,r10
aes_mul_gf28_next_xor:
	shr r11b,1
	jnc aes_mul_gf28_skip
	xor rax,[aes_mul_gf28_scratchpad + rcx - 1]	
aes_mul_gf28_skip:
	loop aes_mul_gf28_next_xor
	epilogue
	ret
	
;;; xtime multiplies one byte representing a polynomial in GF2^8 by x.
;;; Preconditions:
;;; 	rdi is 0 everywhere but dil
;;; Parameters:
;;; 	dil contains the polynomial
;;; Returns:
;;; 	al contains the result of GF2^8 multiplication
xtime:
	prologue
	mov r10,rdi
	shl r10b,1
	jnc xtime_return	;If the carry bit isn't set, we're done.
	or r10w,0x100		;Make sure we preserve the carry bit.
	xor r10w,[gf28_modulo]	;Subtract the modulo value, then we're done.
xtime_return:
	mov rax,r10
	epilogue
	ret

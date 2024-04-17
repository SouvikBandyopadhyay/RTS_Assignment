org 0x7C00					; Directive to hint assembler to calculate adresses starting from 0x7C00
bits 16						; Directive to set mode of operation to 16 bits

%define ENDL 0x0D, 0x0A		; macro to print line end

start:
	jmp main				; because we are writing puts funtion above main, we have to make sure program starts execution from main.

;
;	Prints a string to the screen.
;	Parans:
;		-ds:si points to string
;
puts:
	; save registers we will modify
	push si
	push ax

.loop:
	lodsb					; Loads a bite from the address ds:si into the al register and then increment si by the number of bytes loaded.
	or al, al				; performs bitwise OR between source and destination, to check if next character is null, it will modify the flags register if the result is zero. 
	jz .done				; conditional jump to done varibale when flag is 0
	
	mov ah, 0x0e			; to print a character in tty mode ah needs to be 0e
	mov bh, 0 				; Page number set to zero
	int 0x10				; call bios interupt 10 to print, as al already has the ascii character we need to print

	jmp .loop				; else jump if flag not 0

.done:
	pop ax					; pop registers in reverse order of push
	pop si
	ret
		
main:

	; setup data segment
	mov ax, 0 				; can't write to ds/es directly, therefore writing 0 to ax
	mov ds, ax				; set ds and ex to 0
	mov es, ax

	; setup stack
	mov ss, ax				; set stack sector to 0
	mov sp, 0x7C00 			; set stack pointer to begining of our program because stack grows downwards from where we are loaded in memory

	; print massage
	mov si, msg_hello
	call puts
	
	hlt

.halt:
	jmp .halt				; infinite loop

msg_hello: db 'Hello world!', ENDL, 0 ; string hello world

times 510-($-$$) db 0 		; pad remaining space after the program in the 0th sector with zeros until last 2 bytes
dw 0AA55h					; last 2 bytes are set to signature of OS 

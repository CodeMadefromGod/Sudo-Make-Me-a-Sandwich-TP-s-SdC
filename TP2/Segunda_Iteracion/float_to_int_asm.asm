global float_to_int_asm

section .text

; Firma: int float_to_int_asm(float x)
; En la ABI System V (x86-64 en Linux), el primer argumento (float) llega en el registro xmm0.
; El retorno (int) debe dejarse en eax.

float_to_int_asm:
    ; 1. Prólogo del Stack Frame
    push rbp                ; Guardamos el base pointer anterior
    mov rbp, rsp            ; Establecemos nuestro propio base pointer
    
    ; Reservamos espacio en la pila para variables locales (16 bytes para alinear a 16)
    sub rsp, 16             

    ; 2. Cumplir el requerimiento: Pasar el valor del registro (xmm0) a la memoria de pila
    movss dword [rbp-4], xmm0

    ; 3. Leer de la pila, castear a entero (truncamiento) y guardar en eax
    cvttss2si eax, dword [rbp-4] 

    ; 4. Sumar 1 al resultado para el redondeo/requerimiento
    inc eax                 ; eax = eax + 1

    ; 5. Epílogo del Stack Frame
    mov rsp, rbp            ; Restauramos el stack pointer
    pop rbp                 ; Restauramos el base pointer
    ret                     ; Retornamos a la función invocadora (en C)

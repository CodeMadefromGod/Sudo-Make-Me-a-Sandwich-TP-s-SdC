

## Desafio Final: Modo Protegido 

Basandonos en el codigo proporcionado "common.h", implementamos un codigo en ensamblador "protected_mode.asm" que nos permite entrar en modo protegido y imprimir "Hello World".

El codigo que se realizo es el siguiente:

```assembly
; protected_mode.asm

; Establecemos el modo real de 16 bits y la direccion de memoria

[BITS 16]
[ORG 0x7C00]

start:
    ;deshabilitamos interrupciones
    cli

    ;cargamos la GDT
    lgdt [gdt_descriptor]

    ;activamos el modo protegido activando el bit 0 del registro CR0
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    ;realizamos un salto largo hacia el codigo del modo protegido
    jmp CODE_SEG:protected_mode


;===========================
; GDT
;===========================

gdt_start:

gdt_null:
    dq 0

gdt_code:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00

gdt_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

;===========================
; PROTECTED MODE
;===========================

[BITS 32] ; Modo protegido usa 32 bits
protected_mode:
     ; cargamos los segmentos de datos ya no direcciones
     mov ax, DATA_SEG
     mov ds, ax
     mov es, ax
     mov ss, ax
     mov fs, ax
     mov gs, ax
     
     ; cargamos la direccion de memoria en el stack
     mov ebp, 0x90000 ; no pisamos codigo ni VGA
     mov esp, ebp

     ; Imprimimos "Hello World" en modo protegido
     mov edi, 0xB8000 ; direccion de memoria VGA
     mov esi, mensaje

print_loop:
    lodsb ; esto lo que hace es leer un caracter del string
    cmp al, 0 ; compara con el ultimo valor para ver si es 0
    je halt

    ; color blanco sobre azul
    mov ah, 0x1F
    mov [edi], ax
    add edi, 2

    jmp print_loop

halt:
    jmp $
    
mensaje db "Hello World from Group Sudo Make Me a Sandwich",0

; boot signature
times 510 - ($ - $$) db 0 ; rellena hasta 512 bytes
dw 0xAA55

```

Para compilar usamos el comando:

```bash
 nasm -f bin protected_mode.asm -o protected_mode.bin
```

El cual generara un archivo binario que puede ser usado en qemu.

Luego, para visualizar el resultado en qemu usamos el comando:

```bash
  qemu-system-i386 -drive file=protected_mode.bin,format=raw,if=floppy
```

El resultado que se obtuvo fue:

![alt text](image.png)


### ¿Cómo sería un programa que tenga dos descriptores de memoria diferentes, uno para cada segmento (código y datos) en espacios de memoria diferenciados?

### Cambiar los bits de acceso del segmento de datos para que sea de solo lectura, intentar escribir, ¿Que sucede? ¿Que debería suceder a continuación? (revisar el teórico) Verificarlo con gdb.

### En modo protegido, ¿Con qué valor se cargan los registros de segmento? ¿Porque?
# Trabajo Práctico N°3: Modo Protegido

### Asignatura: Sistemas de Computación

**Facultad de Ciencias Exactas, Físicas y Naturales (UNC)**

---

* **Grupo:** Sudo Make A Sandwich
* **Profesores:** Miguel Angel Solinas y Javier Alejandro Jorge

---

### Integrantes y Contacto

| Nombre y Apellido | Correo Electrónico |
| :--- | :--- |
| **Sergio Andrés Fernández Segovia** | _sergio.fernandez.segovia@mi.unc.edu.ar_ |
| **Enzo Leonel Laura Surco** | _enzo.laura.surco@mi.unc.edu.ar_ |
| **Saqib Daniel Mohammad Cabrejos** | _saqib.mohammad@mi.unc.edu.ar_ |




## 4.- Desafio Final: Modo Protegido 

Basándonos en el código proporcionado "common.h", implementamos un código en ensamblador "protected_mode.asm" que nos permite entrar en modo protegido e imprimir "Hello World".

El código que se realizó es el siguiente:

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

![](https://github.com/SergioAndresF/Sudo-Make-Me-a-Sandwich-TP-s-SdC/blob/TP3/TP3/desafio_final/image.png)


### ¿Cómo sería un programa que tenga dos descriptores de memoria diferentes, uno para cada segmento (código y datos) en espacios de memoria diferenciados?

Para poder responder esta pregunta, hay describir que el CPU usará siempre una misma GDT. Entonces habiendo entendido eso lo que hay que hacer es diferenciar los segmentos base tanto para la parte de codigo como para la parte de datos. Por ejemplo:

```assembly
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
```

En este caso tenemos que:

```assembly
gdt_code:
    dw 0xFFFF ;<---------------- base low
    dw 0x0000
    db 0x00 ;<---------------- la direccion base es 0x00000000
    db 10011010b
    db 11001111b
    db 0x00 ;<---------------- base high

gdt_data:
    dw 0xFFFF
    dw 0x0000 ;<---------------- base low
    db 0x00 ;<---------------- la direccion base es 0x00000000
    db 10010010b
    db 11001111b
    db 0x00 ;<---------------- base high
```
Tanto el segmento de datos como el código de datos tienen la misma base, apuntando a la misma memoria, la respuesta a la pregunta seria lo siguiente:

```assembly
gdt_code:
    dw 0xFFFF ;<---------------- base low
    dw 0x0000
    db 0x00 ;<---------------- la direccion base es 0x00000000
    db 10011010b
    db 11001111b
    db 0x00 ;<---------------- base high

gdt_data:
    dw 0xFFFF
    dw 0x0000 ;<---------------- base low
    db 0x10 ;<---------------- la direccion base es 0x00100000
    db 10010010b
    db 11001111b
    db 0x00 ;<---------------- base high
```

Ahora en este caso el segmento de codigo apuntará a otra región y el segmento de datos a otra teniendo distinta base. Esto es segmentación real. El CPU usa base + offset


### Cambiar los bits de acceso del segmento de datos para que sea de sólo lectura, intentar escribir ¿Qué sucede? ¿Qué debería suceder a continuación? (revisar el teórico) Verificarlo con gdb.

Si la idea es cambiar los permisos del segmento de datos para que sea de solo lectura, hay que modificar el bit de lectura y escritura del descriptor de datos. En este caso, el bit de lectura y escritura es el bit 1. Entonces, si queremos que sea de sólo lectura, hay que poner el bit de lectura y escritura en 0.


```assembly
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
    db 0x10 
    db 10010000b
    db 11001111b
    db 0x00 
```
Haciendo esto cuando el CPU ejecute la instrucción:

```assembly
mov [edi], ax
```
Como el segmento read-only ocasionará un General Protection Fault. Que puede ser observado mediante GDB:

Primero ejecutamos qemu de la siguiente manera para que no inicie el programa hasta mencionarlo con gdb:

```bash
qemu-system-i386 -drive file=protected_mode.bin,format=raw -S -s -no-reboot
```
Esto hara que aun no se inicialice el programa y nos el control para debugguearlo con gdb:

![](https://github.com/SergioAndresF/Sudo-Make-Me-a-Sandwich-TP-s-SdC/blob/TP3/TP3/desafio_final/image-1.png)

 Luego abrimos gdb y nos conectamos al proceso:

```bash
gdb
target remote localhost:1234
```

Colocamos un breakpoint al inicio del programa:

```bash
break *0x7c00
```

En este punto al darle continue deberia poder visualizarse el programa en qemu. Pero en modo real sin saltar al modo protegido. Si continuamos dando "si" instruccion por instruccion deberia llegar a:

```bash
Breakpoint 1 at 0x7c00
(gdb) c
Continuing.

Breakpoint 1, 0x00007c00 in ?? ()
(gdb) si
0x00007c01 in ?? ()
(gdb) si
0x00007c06 in ?? ()
(gdb) si
0x00007c09 in ?? ()
(gdb) si
0x00007c0d in ?? ()
(gdb) si
0x00007c10 in ?? ()
(gdb) si
0x00007c33 in ?? ()
(gdb) si
0x00007c37 in ?? ()
(gdb) si
0x00007c39 in ?? ()
(gdb) si
0x00007c3b in ?? ()
(gdb) si
[Inferior 1 (process 1) exited normally]
```
Esto no quiere decir que el programa no haya sido exitoso en mostrar el mensaje sino debido al error que se genero al intentar escribir en memoria no permitida. El programa termino abruptamente debido al error generado. Podemos observar qemu de esta forma, donde se puede visualizar los registros y memoria:

```bash
qemu-system-i386 -drive file=protected_mode.bin,format=raw -no-reboot -d int
```
El programa no termina normalmente sino que lo cierra abruptamente y en el log se puede visualizar el error que se generó al intentar escribir en memoria no permitida:

```
Servicing hardware INT=0x08
Servicing hardware INT=0x08
Servicing hardware INT=0x08
Servicing hardware INT=0x08
check_exception old: 0xffffffff new 0xd
.
.
.
.
check_exception old: 0x8 new 0xd
```

### En modo protegido. ¿Con qué valor se cargan los registros de segmento? ¿Por qué?


En modo protegido, los registros de segmento no contienen direcciones físicas como en modo real, sino selectores de segmento. Estos selectores son índices que apuntan a entradas en la Global Descriptor Table (GDT) o Local Descriptor Table (LDT). Cada entrada describe un segmento, incluyendo su dirección base, tamaño y permisos de acceso. Esto permite al procesador implementar mecanismos de protección de memoria evitando que los programas rompan memoria, verificando los accesos antes de ejecutarlos y validar permisos del segmento.

```assembly
; cargamos los segmentos de datos ya no direcciones
     mov ax, DATA_SEG
     mov ds, ax
     mov es, ax
     mov ss, ax
     mov fs, ax
     mov gs, ax
```


## 5.- Bonus Track: Introducción a UEFI

### Objetivo
Como punto adicional en este trabajo práctico, se buscó comprender el funcionamiento básico del entorno UEFI y lograr ejecutar un mínimo código a través de una máquina virtual antesala del TP3a. Para ello, se siguieron las instrucciones del siguiente enlace `https://wiki.osdev.org/UEFI_App_Bare_Bones`. Mas detalladamente esta seccion:

![](https://github.com/SergioAndresF/Sudo-Make-Me-a-Sandwich-TP-s-SdC/blob/TP3/TP3/desafio_final/image-2.png)

El procedimiento detallado en este tutorial consistía en la preparación del entorno, compilación del programa, creación de una imagen FAT y, finalmente, la ejecución en QEMU con firmware OVMF.

Como una breve introducción, UEFI es el firmware moderno que reemplaza progresivamente al BIOS tradicional. Entre sus funciones se encuentra inicializar el hardware y permitir la carga de aplicaciones antes de que arranque un sistema operativo. En esta parte del trabajo se buscó comprobar, de manera práctica, cómo una aplicación UEFI puede ser compilada, empaquetada y ejecutada.

### 5.1. Preparación del Entorno
Antes de comenzar, fue necesario instalar todas las herramientas requeridas para desesarrollar y probar aplicaciones UEFI en Linux. Se tratan de paquetes que se emplean para la emulación, firmware, compilación y manipulación de sistemas de archivos FAT.

Los paquetes instalados fueron los siguientes:

- `qemu-system-x86` y `qemu-utils`: Permiten emular la arquitectura x86.
- `ovmf`: Proporciona firmware UEFI para QEMU.
- `gnu-efi`: Incluye cabeceras y bibliotecas específicas para aplicaciones UEFI.
- `binutils-mingw-w64` y `gcc-mingw-w64`: Permiten compilzar y enlazar el código en un formato compatible con UEFI.
- `mtools`: Permite crear y modificar imágenes FAT sin tener que montarlas manualmente.
- `xorriso`: Herramienta auxiliar para manipulación de imágenes.

Para la instalación se emplearon las siguientes líneas de comandos:

```bash
sudo apt update
sudo apt install qemu-system-x86 qemu-utils ovmf gnu-efi mtools xorriso gcc-mingw-w64
binutils-mingw-w64
```
![](https://github.com/SergioAndresF/Sudo-Make-Me-a-Sandwich-TP-s-SdC/blob/TP3/TP3/Bonus%20Track/Imagenes/Entorno.png)   

### 5.2. Verificación del firmware de UEFI en QEMU
Una vez instalado el entorno, se debe verificar que QEMU pueda arrancar con firmware UEFI. A continuación, se muestra el comando utilizado. En dicha línea se usa `pflash` y la razón es que OVMF necesita cargar el firmware como memoria flash, de forma similar a como lo haría una placa real.

```bash
qemu-system-x86_64 \
-drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd \
-drive if=pflash,format=raw,file=/usr/share/OVMF/OVMF_VARS_4M.fd
```

Al ejecutar este comando apareció la pantalla de TianoCore, confirmando que el entorno UEFI funcionaba correctamente.

![](https://github.com/SergioAndresF/Sudo-Make-Me-a-Sandwich-TP-s-SdC/blob/TP3/TP3/Bonus%20Track/Imagenes/Verificacion.png)  

### 5.3. Creación del programa UEFI
En este paso, se crea el código de prueba, el cual se guarda bajo el nombre de `hello.c`. Como se comentó al inicio, se trata de un código simple que, en este caso, muestra un mensaje por pantalla. El código trabajado fue el siguiente:

``` c
#include <efi.h>
#include <efilib.h>

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    InitializeLib(ImageHandle, SystemTable);

    Print(u"Hello World!\r\n");

    Print(u"From Group: Sudo Make Me a Sandwich!\r\n");

    while (1);

    return EFI_SUCCESS;
}
```
En el código anterior se identifica lo siguiente:

- `#include <efi.h>` y `#include <efilib.h>`: Encargados de incluir las definiciones necesarias para programar en UEFI.
- `efi_main()`: Se trata del punto de entrada de un código o aplicación UEFI. Podría ser equivalente asimilarlo a un `main()` de un programa convencional.
- `InitializeLib(ImageHandle, SystemTable)`: Inicializa la biblioteca GNU-EFI y permite usar funciones, en este caso en particular, `Print`.
- `Print(u"Hello World!\r\n")` y `Print(u"From Group: Sudo Make Me a Sandwich!\r\n")`: Se llama a la función para imprimir los mensajes en la consola del firmware.
- `while (1)`: Busca mantener la ejecución del programa para evitar que el mensaje desaparezca.
- `return EFI_SUCCESS`: Indica que la ejecución terminó correctamente; sin embargo, en este caso nunca llegaría a eso debido al `while(1)`.

### 5.4. Compilación del código
Esta operación implica transformar `hello.c` en un archivo objeto `hello.o`. Seguidamente, se comparte la línea de comando para tal efecto.

``` bash
gcc \
-I/usr/include/efi \
-I/usr/include/efi/x86_64 \
-fpic \
-ffreestanding \
-fno-stack-protector \
-mno-red-zone \
-c hello.c -o hello.o
```
Del bloque anterior, se observa:

- `-ffreestanding`: Indica que el código no depende de un SO convencional.
- `-fno-stack-protecto`: Desactiva protecciones pensadas para programas de usuario normales, las cuales, en este caso no son apropiadas.
- `-mno-red-zone`: Deshabilita la red zone.
- `-c`: Complia sin enlazar, se genera el archivo objeto.

![](https://github.com/SergioAndresF/Sudo-Make-Me-a-Sandwich-TP-s-SdC/blob/TP3/TP3/Bonus%20Track/Imagenes/Compilacion.png)  


### 5.5. Enlazado con GNE-EFI
Debido a que el archivo `hello.o` no es suficiente para generar una aplicación UEFI, resulta necesario enlazarlo con las bibliotecas de GNU-EFI y con el archivo de arranque específico para este retorno, obteniendo `hello.so`.

Se trabajó con el siguiente comando:

``` bash
gcc \-nostdlib \
-Wl,-dll \
-Wl,-shared \
-Wl,-Bsymbolic \
-Wl,-T,/usr/lib/elf_x86_64_efi.lds \
-Wl,--entry,_start \
/usr/lib/crt0-efi-x86_64.o \
hello.o \
-L/usr/lib \
-lefi -lgnuefi \
-o hello.so
```
- `-nostdlib`: Evita enlazar la biblioteca estándar de C.
- `crt0-efi-x86_64.o`: Archivo de arranque que prepara la ejecución en UEFI.
- `-lefi`y `lgnuefi`: Enlazan las bibliotecas necesarias para utilizar servicios del firmware.
- `-T,/usr/lib/elf_x86_64_efi.lds`: Usa el script de enlace específico para aplicaciones EFI.
- `-o hello.so`: Crea el binario enlazado.

![](https://github.com/SergioAndresF/Sudo-Make-Me-a-Sandwich-TP-s-SdC/blob/TP3/TP3/Bonus%20Track/Imagenes/Enlace.png)  

Este paso es bastante importante ya que UEFI no ejecuta directamente un objeto compilado, sino que primero necesita un binario enlazado con la estructura adecuada.

### 5.6. Conversión al formato EFI
Pese a que el archivo `hello.so` se encuetra enlazado, aún no presenta el formato final que UEFI espera. En ese sentido, se utiliza `objcopy` para su conversión.

``` bash
objcopy \
-j .text \
-j .sdata \
-j .data \
-j .dynamic \
-j .dynsym \
-j .rel \
-j .rela \
-j .reloc \
--target efi-app-x86_64 \
hello.so BOOTX64.EFI
```
- `--targe efi-app-x86_64`: Se ocupa de convertir el archivo al formato que el firmware reconoce como aplicación EFI.

![](https://github.com/SergioAndresF/Sudo-Make-Me-a-Sandwich-TP-s-SdC/blob/TP3/TP3/Bonus%20Track/Imagenes/Creacion.png) 

Posteriormente, se utiliza `file BOOTX64.EFI` para verificar que el formato sea correcto, esto es, `PE32+ executable (EFI application)`.

![](https://github.com/SergioAndresF/Sudo-Make-Me-a-Sandwich-TP-s-SdC/blob/TP3/TP3/Bonus%20Track/Imagenes/Uefi.png) 

### 5.7. Creación de la imagen FAT
Para que UEFI pueda arrancar la aplicación automáticamente, el archivo debe ubicarse en la ruta estándar. Para ello, en primera instancia, se debe crear una imagen vacía para, posteriormente, formatearla.

Se muestra el bloque de líneas de comando que se utilizaron:

``` bash
dd if=/dev/zero of=fat.img bs=1M count=64
mformat -i fat.img ::
mmd -i fat.img ::/EFI
mmd -i fat.img ::/EFI/BOOT
mcopy -i fat.img BOOTX64.EFI ::/EFI/BOOT/
```

- `dd if=/dev/zero of=fat.img bs=1M count=64`: Crea un archivo de 64MB lleno de ceros, aún no es un sistema de archivos, sino un contenedor vacío.
- `mformat -i fat.img ::`: Convierte el archivo en un sistema FAT válido. Es importante esta operación ya que caso contrario, `mtools` no podrá reconocer la imagen.
- `mmd -i fat.img ::/EFI` y `mmd -i fat.img ::/EFI/BOOT`: Estos comandos crean los directorios que UEFI busca de forma predeterminada al arrancar.
- `mcopy -i fat.img BOOTX64.EFI ::/EFI/BOOT/`: EL archivo UEFI pasa a ubicarse en la ruta exacta esperada por el firmware.

![](https://github.com/SergioAndresF/Sudo-Make-Me-a-Sandwich-TP-s-SdC/blob/TP3/TP3/Bonus%20Track/Imagenes/Fat.png) 

### 5.8. Ejecución en QEMU
Una vez lista la imagen FAT, se vuelve a arrancar QEMU con OVMF, con el siguiente comando:

``` bash
sudo qemu-system-x86_64 \
-drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE_4M.fd \
-drive if=pflash,format=raw,file=fresh_vars.fd \
-drive file=fat.img,format=raw
```
A continuación, se muestra el resultado:

![](https://github.com/SergioAndresF/Sudo-Make-Me-a-Sandwich-TP-s-SdC/blob/TP3/TP3/Bonus%20Track/Imagenes/Resultado.png) 

### 5.9. Resumen de las actividades
1) escribir hello.c
2) gcc -> hello.o
3) link -> hello.so
4) objcopy -> BOOTX64.EFI
5) crear fat.img
6) copiar BOOTX64.EFI dentro de fat.img
7) arrancar QEMU con fat.img

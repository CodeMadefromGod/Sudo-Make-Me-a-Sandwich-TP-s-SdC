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

---

## Desafio: UEFI y coreboot

### ¿Qué es UEFI? ¿Cómo puedo usarlo?
**UEFI (Unified Extensible Firmware Interface)** es el estándar moderno que reemplaza a la BIOS tradicional. A diferencia de la BIOS, que está atada a la arquitectura de 16 bits y se comunica mediante interrupciones de software, UEFI funciona como un sistema operativo en miniatura de 32 o 64 bits. Tiene soporte nativo para redes, interfaces gráficas complejas y puede leer directamente sistemas de archivos como FAT32 dentro de discos particionados en formato GPT.

En la práctica, se utiliza desarrollando "Aplicaciones EFI" (archivos con extensión ```.efi```, típicamente programados en C o Rust). En lugar de usar instrucciones puras en ensamblador como ```int $0x10```, se llama a las interfaces C estructuradas que expone la API del firmware antes de que se delegue el control al SO.

Dentro de los "UEFI Runtime Services" (servicios diseñados para permanecer residentes en memoria incluso después de que arranque el kernel de Linux), una función muy crítica es ```GetVariable()``` o ```SetVariable()```. Se utiliza para leer o escribir variables no volátiles (NVRAM) a nivel de placa base, como el orden de booteo o configuraciones de hardware. Otra es ```GetTime()```, para interactuar directamente con el Real-Time Clock (RTC) del hardware.

### Casos recientes de bugs de UEFI explotables

**LogoFAIL (CVE-2023-5058 y relacionados):** Descubierto a finales de 2023. El código de la UEFI encargado de mostrar el logotipo del fabricante de la placa base durante la fase de arranque (fase DXE) carecía de validaciones de seguridad. Un atacante puede reemplazar el archivo de imagen original en la partición ESP por una imagen alterada. Al parsearla, se inyecta código malicioso que se ejecuta antes que el SO, saltándose por completo la verificación criptográfica de Secure Boot.

**BlackLotus (CVE-2022-21894):** Es un infame bootkit moderno. Fue el primero en la naturaleza diseñado explícitamente para saltarse el Secure Boot en sistemas Windows 11 totalmente actualizados. Explotaba una vulnerabilidad en la lista de revocación de firmas durante el proceso de arranque, tomando el control total del flujo y desactivando las protecciones antimalware del kernel antes de que estas pudieran siquiera iniciarse.

### ¿Qué es Converged Security and Management Engine (CSME), the Intel Management Engine BIOS Extension (Intel MEBx)?

**Intel CSME:** Anteriormente conocido como "Intel Management Engine" (IME), es literalmente una computadora oculta dentro de tu computadora. Es un subsistema embebido en los chipsets de Intel que tiene su propio microprocesador independiente (generalmente basado en arquitecturas MINIX), su propia memoria, y acceso total al hardware (Direct Memory Access, control de red). Funciona sin importar si el SO está encendido, suspendido o apagado. Está diseñado para ofrecer servicios de DRM (Gestión de Derechos Digitales), TPM (Módulo de Plataforma Segura) y administración remota corporativa.

**Intel MEBx (Management Engine BIOS Extension):** Es la interfaz de configuración visual para este subsistema. Es el menú clásico que permite a los administradores de TI configurar los parámetros de red del CSME, establecer contraseñas o habilitar herramientas como Intel Active Management Technology (AMT).

### ¿Qué es coreboot? Ventajas y productos

Es un proyecto de firmware de código abierto cuyo único objetivo es reemplazar las BIOS/UEFI propietarias y cerradas. Su filosofía es hacer la menor cantidad de cosas posibles: inicializar la placa base y la memoria RAM, e inmediatamente entregar el control a un payload (que puede ser un gestor como SeaBIOS, GRUB, o el kernel de Linux directamente).

**Ventajas:**

- **Velocidad extrema:** Al deshacerse de los cientos de módulos inútiles que los fabricantes empaquetan en la UEFI comercial, reduce los tiempos de inicio del hardware a milisegundos.

- **Auditoría y Seguridad:** Al ser de código abierto, la comunidad puede auditarlo en busca de puertas traseras, reduciendo casi a cero la superficie de ataque para bootkits.

- **Desactivación del Intel ME:** Coreboot permite a herramientas como me_cleaner neutralizar y despojar al subsistema Intel CSME/ME de casi todo su código, devolviéndole la privacidad y el control total de la máquina al usuario.

**Productos que lo incorporan:**

- **Google Chromebooks:** Prácticamente todos los dispositivos de la línea ChromeOS utilizan coreboot modificado por Google para asegurar arranques casi instantáneos.

- Laptops de nicho para desarrolladores e ingenieros en Linux, como las de System76 (modelos Lemur, Oryx), Star Labs y los dispositivos orientados a privacidad de Purism (línea Librem).

## Desafio: Linker

### Run image in QEMU

![imagen](https://hackmd.io/_uploads/B18RlU3p-g.png)
Demostración del ciclo completo de desarrollo y ejecución de un MBR personalizado. En la terminal superior se observa el ensamblado (as) y enlazado (ld con formato binario plano) del código fuente para generar la imagen main.img de 512 bytes. En la ventana inferior, la máquina virtual QEMU emula el proceso de arranque, donde SeaBIOS carga exitosamente el sector generado e imprime en pantalla el mensaje del equipo.

### Run image in Hardware
![imagen](https://hackmd.io/_uploads/By5FYI2a-x.png)

![imagen](https://hackmd.io/_uploads/H18XOw3pWe.png)

![imagen](https://hackmd.io/_uploads/HyiQy-pT-e.png)

Para cumplir con los estrictos requisitos de validación de la BIOS Legacy InsydeH20, se descartó la escritura cruda del sector 0 completo, ya que esto corrompe la estructura lógica del disco. En su lugar, se implementó un procedimiento en dos fases. Primero, se generó manualmente una tabla de particiones MBR válida utilizando ```fdisk```, marcando la partición primaria como activa (flag **0x80**). Posteriormente, se realizó una inyección del código ejecutable (```main.img```) utilizando el comando ```dd``` con los parámetros ```bs=446 count=1 conv=notrunc```. Esta configuración permitió copiar exclusivamente las instrucciones en el Bootstrap code area (los primeros 446 bytes), mientras que el parámetro ```notrunc``` evitó que la herramienta truncara el archivo o sobrescribiera los 66 bytes restantes. De esta manera, se preservó intacta tanto la tabla de particiones recién creada como la firma mágica **0x55AA**, logrando exitosamente que la BIOS reconozca al USB como una unidad booteable válida.

### Comparacion objdump vs hexdump

#### objdump
```bash=
anpanman17@Sony-Vaio:~/Escritorio/SdC-2026/MBR/hello-world$ objdump -S main.o

main.o:     formato del fichero elf64-x86-64


Desensamblado de la sección .text:

0000000000000000 <loop-0x5>:
.code16
    mov $msg, %si
   0:	be 00 00 b4 0e       	mov    $0xeb40000,%esi

0000000000000005 <loop>:
    mov $0x0e, %ah
loop:
    lodsb
   5:	ac                   	lods   %ds:(%rsi),%al
    or %al, %al
   6:	08 c0                	or     %al,%al
    jz halt
   8:	74 04                	je     e <halt>
    int $0x10
   a:	cd 10                	int    $0x10
    jmp loop
   c:	eb f7                	jmp    5 <loop>

000000000000000e <halt>:
halt:
    hlt
   e:	f4                   	hlt

000000000000000f <msg>:
   f:	48                   	rex.W
  10:	65 6c                	gs insb (%dx),%es:(%rdi)
  12:	6c                   	insb   (%dx),%es:(%rdi)
  13:	6f                   	outsl  %ds:(%rsi),(%dx)
  14:	20 57 6f             	and    %dl,0x6f(%rdi)
  17:	72 6c                	jb     85 <msg+0x76>
  19:	64 20 64 65 73       	and    %ah,%fs:0x73(%rbp,%riz,2)
  1e:	64 65 20 55 53       	fs and %dl,%gs:0x53(%rbp)
  23:	42 20 66 74          	rex.X and %spl,0x74(%rsi)
  27:	2e 20 42 49          	cs and %al,0x49(%rdx)
  2b:	4f 53                	rex.WRXB push %r11
  2d:	20 49 6e             	and    %cl,0x6e(%rcx)
  30:	73 79                	jae    ab <msg+0x9c>
  32:	64 65 48 32 30       	fs rex.W xor %gs:(%rax),%sil
  37:	2c 20                	sub    $0x20,%al
  39:	73 6f                	jae    aa <msg+0x9b>
  3b:	6d                   	insl   (%dx),%es:(%rdi)
  3c:	6f                   	outsl  %ds:(%rsi),(%dx)
  3d:	73 20                	jae    5f <msg+0x50>
  3f:	65 6c                	gs insb (%dx),%es:(%rdi)
  41:	20 67 72             	and    %ah,0x72(%rdi)
  44:	75 70                	jne    b6 <msg+0xa7>
  46:	6f                   	outsl  %ds:(%rsi),(%dx)
  47:	20 22                	and    %ah,(%rdx)
  49:	53                   	push   %rbx
  4a:	75 64                	jne    b0 <msg+0xa1>
  4c:	6f                   	outsl  %ds:(%rsi),(%dx)
  4d:	20 4d 61             	and    %cl,0x61(%rbp)
  50:	6b 65 20 41          	imul   $0x41,0x20(%rbp),%esp
  54:	20 53 61             	and    %dl,0x61(%rbx)
  57:	6e                   	outsb  %ds:(%rsi),(%dx)
  58:	64 77 69             	fs ja  c4 <msg+0xb5>
  5b:	63 68 22             	movsxd 0x22(%rax),%ebp
  5e:	21 00                	and    %eax,(%rax)
```

#### hexdump
```bash=
anpanman17@Sony-Vaio:~/Escritorio/SdC-2026/MBR/hello-world$ hd main.img
00000000  be 0f 7c b4 0e ac 08 c0  74 04 cd 10 eb f7 f4 48  |..|.....t......H|
00000010  65 6c 6c 6f 20 57 6f 72  6c 64 20 64 65 73 64 65  |ello World desde|
00000020  20 55 53 42 20 66 74 2e  20 42 49 4f 53 20 49 6e  | USB ft. BIOS In|
00000030  73 79 64 65 48 32 30 2c  20 73 6f 6d 6f 73 20 65  |sydeH20, somos e|
00000040  6c 20 67 72 75 70 6f 20  22 53 75 64 6f 20 4d 61  |l grupo "Sudo Ma|
00000050  6b 65 20 41 20 53 61 6e  64 77 69 63 68 22 21 00  |ke A Sandwich"!.|
00000060  66 2e 0f 1f 84 00 00 00  00 00 66 2e 0f 1f 84 00  |f.........f.....|
00000070  00 00 00 00 66 2e 0f 1f  84 00 00 00 00 00 66 2e  |....f.........f.|
00000080  0f 1f 84 00 00 00 00 00  66 2e 0f 1f 84 00 00 00  |........f.......|
00000090  00 00 66 2e 0f 1f 84 00  00 00 00 00 66 2e 0f 1f  |..f.........f...|
000000a0  84 00 00 00 00 00 66 2e  0f 1f 84 00 00 00 00 00  |......f.........|
000000b0  66 2e 0f 1f 84 00 00 00  00 00 66 2e 0f 1f 84 00  |f.........f.....|
000000c0  00 00 00 00 66 2e 0f 1f  84 00 00 00 00 00 66 2e  |....f.........f.|
000000d0  0f 1f 84 00 00 00 00 00  66 2e 0f 1f 84 00 00 00  |........f.......|
000000e0  00 00 66 2e 0f 1f 84 00  00 00 00 00 66 2e 0f 1f  |..f.........f...|
000000f0  84 00 00 00 00 00 66 2e  0f 1f 84 00 00 00 00 00  |......f.........|
00000100  66 2e 0f 1f 84 00 00 00  00 00 66 2e 0f 1f 84 00  |f.........f.....|
00000110  00 00 00 00 66 2e 0f 1f  84 00 00 00 00 00 66 2e  |....f.........f.|
00000120  0f 1f 84 00 00 00 00 00  66 2e 0f 1f 84 00 00 00  |........f.......|
00000130  00 00 66 2e 0f 1f 84 00  00 00 00 00 66 2e 0f 1f  |..f.........f...|
00000140  84 00 00 00 00 00 66 2e  0f 1f 84 00 00 00 00 00  |......f.........|
00000150  66 2e 0f 1f 84 00 00 00  00 00 66 2e 0f 1f 84 00  |f.........f.....|
00000160  00 00 00 00 66 2e 0f 1f  84 00 00 00 00 00 66 2e  |....f.........f.|
00000170  0f 1f 84 00 00 00 00 00  66 2e 0f 1f 84 00 00 00  |........f.......|
00000180  00 00 66 2e 0f 1f 84 00  00 00 00 00 66 2e 0f 1f  |..f.........f...|
00000190  84 00 00 00 00 00 66 2e  0f 1f 84 00 00 00 00 00  |......f.........|
000001a0  66 2e 0f 1f 84 00 00 00  00 00 66 2e 0f 1f 84 00  |f.........f.....|
000001b0  00 00 00 00 66 2e 0f 1f  84 00 00 00 00 00 66 2e  |....f.........f.|
000001c0  0f 1f 84 00 00 00 00 00  66 2e 0f 1f 84 00 00 00  |........f.......|
000001d0  00 00 66 2e 0f 1f 84 00  00 00 00 00 66 2e 0f 1f  |..f.........f...|
000001e0  84 00 00 00 00 00 66 2e  0f 1f 84 00 00 00 00 00  |......f.........|
000001f0  66 2e 0f 1f 84 00 00 00  00 00 0f 1f 40 00 55 aa  |f...........@.U.|
00000200
```

#### Comparación y ubicación del programa dentro de la imagen

Al comparar la salida relocalizable de ```objdump``` con el binario crudo mostrado por ```hd```, podemos verificar exactamente cómo el Linker ubicó nuestro programa dentro de la estructura de 512 bytes del MBR:

![image](https://hackmd.io/_uploads/SJQW1KapWx.png)

- **Código útil y datos (Offset 0x0000 al 0x005F):** En la salida de ```objdump```, observamos que nuestro código ensamblador y la cadena de texto ocupan hasta el offset **0x5E**. Al revisar ```hd```, confirmamos que este bloque exacto de instrucciones y datos fue colocado al inicio de la imagen, ocupando la sección destinada al ```Bootstrap code area```.

- **Relleno de alineación (Offset 0x0060 al 0x01FD):** A partir del offset **0x60**, ```hd``` muestra una repetición de bytes **(66 2e 0f 1f...)**. Estos corresponden a instrucciones NOP (secuencias que no alteran el estado del procesador) insertadas automáticamente por el Linker. Esto es el resultado directo de la directiva ```. = 0x1FE;``` en nuestro linker script, la cual obliga al ensamblador a rellenar el espacio vacío hasta alcanzar exactamente el byte 510 de la imagen.

- **Firma de arranque (Offset 0x01FE al 0x01FF):** En la última línea de ```hd``` (que comienza en el offset **0x1F0**), los dos bytes finales son **55 aa**. Esto no es casualidad, sino la traducción de la directiva ```SHORT(0xAA55)```, escrita en formato Little Endian. Esta firma mágica  es el requisito indispensable a nivel de arquitectura para que la BIOS reconozca el sector como booteable y decida cederle el control a nuestro programa.

#### ¿Qué es un linker? ¿Qué hace?
Un **linker** (o enlazador) es una herramienta del sistema que toma uno o más archivos de objeto (como el ```main.o``` generado por el ensamblador) y los combina para crear un único archivo ejecutable o imagen binaria .

Su trabajo principal es la **relocalización y resolución de símbolos**. Cuando el ensamblador traduce tu código fuente a lenguaje máquina, asigna direcciones relativas (empezando desde el offset 0x0) porque desconoce el lugar físico de la RAM donde se ejecutará el programa. El linker toma ese código "flotante", lo ubica en un mapa de memoria definitivo y reescribe todas las referencias incompletas (como el puntero a la variable ```msg```) para que apunten a las direcciones absolutas correctas.

#### ¿Qué es la dirección que aparece en el script del linker? ¿Por qué es necesaria?
La dirección ```. = 0x7c00;``` que aparece en el script ```(link.ld)``` indica la ubicación física exacta en la memoria RAM donde la BIOS carga tradicionalmente el primer sector de arranque (MBR).

Es **estrictamente necesaria** porque el programa contiene instrucciones que acceden a posiciones de memoria absolutas. Al definir este origen en el script, le estamos dando al linker un punto de referencia espacial. Esto permite que el linker calcule matemáticamente dónde caerá cada variable e instrucción una vez que la BIOS copie los 512 bytes del pendrive a la RAM. Si omitimos esta dirección, el linker asumirá que el programa arranca en ```0x0000```, provocando que el procesador intente leer el mensaje de una parte equivocada de la memoria.

#### ¿Para qué se utiliza la opción --oformat binary en el linker?
Esta opción fuerza al linker a generar un archivo binario "plano" o crudo (flat binary), despojándolo de cualquier cabecera, tabla de símbolos o metadatos del sistema operativo.

Por defecto, los linkers en Linux generan ejecutables en formato ELF (Executable and Linkable Format). Sin embargo, la BIOS no es un sistema operativo y no sabe interpretar qué significa una cabecera ELF; simplemente lee el sector 0 del disco y comienza a ejecutar ciegamente los bytes que encuentra allí uno tras otro. Al usar ```--oformat binary```, garantizamos que la salida ```main.img``` contenga única y exclusivamente tu código de máquina puro y la estructura exacta de 512 bytes exigida por el MBR.

## Depuracion de ejecutables con llamadas a bios int

### Fase 1: Conexión Inicial de depuracion remota entre Qemu y GDB, Reset Vector (0xFFF0)
![imagen](https://hackmd.io/_uploads/Skd7Cv6pZg.png)

### Fase 2: Breakpoint 1 (0x7c00) y codigo assembly del MBR
![imagen](https://hackmd.io/_uploads/Hycgb_aTbl.png)

![imagen](https://hackmd.io/_uploads/rkRvZua6Zl.png)

Una vez establecido el breakpoint en la dirección de carga de la BIOS **(0x7C00)**, el depurador GDB detiene la ejecución justo antes de la primera instrucción de nuestro código. Al analizar la vista de desensamblado desde gdb, se observa un fenómeno particular característico de la depuración a bajo nivel: **la desalineación de decodificación**.

Aunque el código fuente fue explícitamente compilado para procesadores de 16 bits mediante la directiva 
```.code16```, el entorno del depurador (ejecutándose en un host de 64 bits) intenta decodificar la memoria en bloques más grandes, generando "falsos positivos" visuales en las instrucciones:

- **Fusión de Mnemónicos (Offset 0x7c00):** En el código fuente, las dos primeras instrucciones son de 16 bits (```mov $msg, %si``` y ```mov $0x0e, %ah```). Sin embargo, GDB las agrupa e interpreta como una única instrucción moderna que opera sobre un registro extendido de 32 bits: ```mov $0xeb47c0f,%esi```. A nivel de opcodes (código máquina), los bytes en la RAM son exactamente los correctos, pero la herramienta los muestra agrupados bajo una sintaxis incorrecta.

- **Sincronización del flujo (Offset 0x7c05 a 0x7c0e):** A pesar del error inicial, a partir del offset **0x7c05** el desensamblador logra realinearse con los bytes correctos, mostrando perfectamente la lógica de nuestro bucle (```lods, or, je, int $0x10, jmp y hlt```).

- **Datos interpretados como instrucciones (Offset 0x7c0f en adelante):** Al llegar a la declaración de la cadena de texto (```.asciz "Hello World..."```), el depurador continúa su lectura asumiendo que los caracteres ASCII son instrucciones ejecutables. Por ejemplo, la letra 'H' (0x48 en ASCII) es interpretada como el prefijo de instrucción ```rex.W```.

### Fase 3: Estrategia de Depuración y Control de Flujo
Para validar el correcto funcionamiento del bucle de impresión en Modo Real, se definió una estrategia de depuración en GDB basada en la colocación de tres puntos de interrupción (breakpoints) estratégicos. Esto permitió aislar la ejecución de nuestro código, evitando adentrarse en las subrutinas de la interrupción de video de la BIOS (```int 0x10```).

#### 1. Punto de entrada absoluto (br *0x7c00)
La primera captura demuestra la pausa inicial exactamente en la dirección de memoria donde la BIOS vuelca el MBR. En este estado, el emulador se encuentra suspendido ("Booting from Hard Disk...") un milisegundo antes de que el procesador ejecute la instrucción de carga del puntero de la cadena de texto (```mov $msg, %si```).

![imagen](https://hackmd.io/_uploads/HktXmupTbl.png)

#### 2. Aislamiento de la iteración (br *0x7c0c)
La segunda captura ilustra la impresión del primer carácter. El breakpoint fue colocado intencionalmente en la instrucción ```jmp loop``` (offset **0x7c0c**), inmediatamente después de la llamada a la interrupción de la BIOS.
![imagen](https://hackmd.io/_uploads/ryddm_ppZe.png)

#### 3. Condición de salida y detención (br *0x7c0e)
La tercera captura exhibe la finalización exitosa del programa. El procesador ha alcanzado la instrucción ```hlt``` (offset **0x7c0e**), congelando el sistema tras imprimir el mensaje completo.
![imagen](https://hackmd.io/_uploads/B1RzEd6abe.png)

---



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

![](https://github.com/Sachan-2512/Sudo-Make-Me-a-Sandwich-TP-s-SdC/blob/TP3/TP3/desafio_final/image.png)


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

![](https://github.com/Sachan-2512/Sudo-Make-Me-a-Sandwich-TP-s-SdC/blob/TP3/TP3/desafio_final/image-1.png)

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
Como punto adicional en este trabajo práctico y con la intencion de investigar acerca de UEFI antes del TP3-a, se buscó comprender el funcionamiento básico del entorno UEFI y lograr ejecutar un mínimo código a través de una máquina virtual. Para ello, se siguieron las instrucciones del siguiente enlace `https://wiki.osdev.org/UEFI_App_Bare_Bones`. Mas detalladamente esta seccion:

![](https://github.com/Sachan-2512/Sudo-Make-Me-a-Sandwich-TP-s-SdC/blob/TP3/TP3/desafio_final/image-2.png)

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

# Trabajo Práctico N°3.a: Entorno UEFI, Desarrollo y Análisis de Seguridad

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


*Objetivo General: Comprender la arquitectura de la Interfaz de Firmware Extensible Unificada (UEFI) como un entorno pre-sistema operativo, desarrollar binarios nativos, entender su formato y ejecutar rutinas tanto en entornos emulados como en hardware físico (bare metal).*

## Preparación del Entorno (Linux)

Para comenzar con el laboratorio, se instalaron las dependencias necesarias incluyendo el emulador QEMU, el firmware OVMF y las herramientas de compilación cruzada. Posteriormente, se procedió a la instalación manual/configuración de Ghidra para el análisis inverso.

![image](https://hackmd.io/_uploads/S1ZTzzXR-g.png)

![image](https://hackmd.io/_uploads/BJqsNfX0bg.png)


## Trabajo Práctico 1: Exploración del entorno UEFI y la Shell



### 1.1 Arranque en el entorno virtual

Iniciamos el entorno virtual utilizando QEMU, pasándole como parámetro el binario del firmware OVMF. Al no especificar un disco de arranque, el sistema nos deja directamente en la UEFI Interactive Shell.

![image](https://hackmd.io/_uploads/Hk93rfQAWe.png)




### 1.2 Exploración de Dispositivos (Handles y Protocolos)

A continuación, utilizamos comandos propios de la Shell de UEFI para listar los dispositivos mapeados y la base de datos de Handles. 
![image](https://hackmd.io/_uploads/rk5QyN7AZe.png)

Nota: Al estar emulando sin almacenamiento adjunto, el comando map expone únicamente el dispositivo de bloques BLK0 sin sistema de archivos FS0.
![image](https://hackmd.io/_uploads/HyZIy47Rbl.png)


![image](https://hackmd.io/_uploads/B1W_k4mRbl.png)


![image](https://hackmd.io/_uploads/ByPqJV7CWg.png)

**Pregunta de Razonamiento 1: Al ejecutar el comando map y dh, vemos protocolos e identificadores en lugar de puertos de hardware fijos. ¿Cuál es la ventaja de seguridad y compatibilidad de este modelo frente al antiguo BIOS?**

**Respuesta:** Este modelo implementa una abstracción total del hardware. En lugar de comunicarse directamente con puertos físicos específicos, el firmware interactúa mediante interfaces estandarizadas (Protocolos). Esto aporta una gran ventaja de compatibilidad, ya que el mismo binario UEFI puede interactuar con un dispositivo sin importarle si está conectado por SATA, USB o PCIe. A nivel de seguridad, al evitar el acceso directo a los componentes físicos, se previenen conflictos de hardware y se facilita el control sobre qué ejecutable tiene permisos para interactuar con qué dispositivo.

## 1.3  Análisis de Variables Globales (NVRAM)

En esta fase interactuamos con la NVRAM, volcando la configuración actual del sistema e inyectando una variable personalizada para comprobar la persistencia de la configuración en la etapa BDS.

![image](https://hackmd.io/_uploads/ByxOxN7Abl.png)
![image](https://hackmd.io/_uploads/ryo9lEQC-e.png)


![image](https://hackmd.io/_uploads/HJjzMVX0Wx.png)


**Pregunta de Razonamiento 2: Observando las variables Boot#### y BootOrder, ¿cómo determina el Boot Manager la secuencia de arranque?**

**Respuesta:** El Boot Manager utiliza un sistema de punteros secuenciales. Primero, lee el contenido de la variable BootOrder, la cual contiene un arreglo numérico con la prioridad de arranque (por ejemplo, 0000, 0002). Luego, toma el primer valor de esa lista y busca su variable correspondiente (ej. Boot0000). Dentro de esta última, lee la ruta específica (Device Path) que apunta al ejecutable .efi a iniciar. Si ese dispositivo no está disponible, pasa al siguiente número en la lista de BootOrder.

## 1.4  Footprinting de Memoria y Hardware

Finalmente, extrajimos el mapa de memoria actual y listamos los dispositivos PCI y drivers cargados en el entorno pre-SO.

![image](https://hackmd.io/_uploads/r1THvEQC-x.png)
![image](https://hackmd.io/_uploads/rJBuDV7CZe.png)
![image](https://hackmd.io/_uploads/rkVcPVQAbl.png)


![image](https://hackmd.io/_uploads/r1ugyI7C-x.png)


![image](https://hackmd.io/_uploads/By4PJUm0bx.png)

![image](https://hackmd.io/_uploads/SJlF1U7Cbg.png)

**Pregunta de Razonamiento 3: En el mapa de memoria (memmap), existen regiones marcadas como RuntimeServicesCode. ¿Por qué estas áreas son un objetivo principal para los desarrolladores de malware (Bootkits)?**

**Respuesta:** La criticidad de las regiones RuntimeServices radica en que no se borran de la memoria una vez que el Sistema Operativo (Windows o Linux) toma el control mediante la función ExitBootServices(). Si un desarrollador de malware (Bootkit) logra inyectar código en estas áreas, se garantiza una persistencia absoluta. El código malicioso continuará ejecutándose por debajo del SO con los máximos privilegios de hardware (como el modo SMM o Ring -2), volviéndose completamente indetectable para los motores de antivirus convencionales.


## Trabajo Práctico 2: Desarrollo, compilación y análisis de seguridad

### 2.1. Objetivo
"Crear una aplicación nativa UEFI en C, entender el formato PE/COFF y analizar cómo un descompilador interpreta opcodes a nivel de firmware."

### 2.2. Desarrollo de la Aplicación
En primera instancia, se crea el archivo `aplicacion.c` y el código contenido en dicho archivo es el que se muestra a continuación.

``` C 
#include <efi.h>
#include <efilib.h>

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {

    InitializeLib(ImageHandle, SystemTable);

    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Iniciando analisis de seguridad...\r\n");
    
    // Inyección de un software breakpoint (INT3)
    unsigned char code[] = { 0xCC };
    
    if (code[0] == 0xCC) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Breakpoint estatico alcanzado.\r\n");
    }
    
    return EFI_SUCCESS;
}
```

### 2.3. Compilación a Formato PE/COFF
El procedimiento a seguir es:
- Compilar el archivo `aplicacion.c` para obtener el archivo objeto `aplicacion.o`.

```bash
gcc \
-I/usr/include/efi \
-I/usr/include/efi/x86_64 \
-I/usr/include/efi/protocol \
-fpic \
-ffreestanding \
-fno-stack-protector \
-fno-strict-aliasing \
-fshort-wchar \
-mno-red-zone \
-maccumulate-outgoing-args \
-Wall \
-c -o aplicacion.o aplicacion.c
```

![Compilacion](https://hackmd.io/_uploads/rJwlulE0-l.png)

- Enlazar el archivo objeto `aplicacion.o` para obtener el ejecutable `aplicacion.so`.
 
```bash
ld -shared \
-Bsymbolic \
-L/usr/lib \
-L/usr/lib/efi \
-T /usr/lib/elf_x86_64_efi.lds \
/usr/lib/crt0-efi-x86_64.o \
aplicacion.o -o aplicacion.so \
-lefi -lgnuefi
```

![Enlazado](https://hackmd.io/_uploads/BJwMdeNA-e.png)

- Convertir el formato de `aplicacion.so` al esperado por UEFI `aplicacion.efi`.

```bash
objcopy \
-j .text \
-j .sdata \
-j .data \
-j .dynamic \
-j .dynsym \
-j .rel \
-j .rela \
-j .rel.* \
-j .rela.* \
-j .reloc \
--target=efi-app-x86_64 aplicacion.so aplicacion.efi
```

![Formato](https://hackmd.io/_uploads/Hyx7ulNCWl.png)

- Verificación a través de `file aplicacion.efi`.

![Ejecutable](https://hackmd.io/_uploads/r1h7Ox4Rbl.png)

### 2.3. Análisis de Metadatos y Decompilación

## Trabajo Práctico 3: Ejecución en Hardware Físico (Bare Metal)
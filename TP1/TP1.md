# Trabajo Practico N°1

### Asignatura: Sistemas de Computacion

**Facultad de Ciencias Exactas, Físicas y Naturales (UNC)**

---

* **Grupo:** Sudo Make A Sandwich
* **Profesores:** Miguel Angel Solinas y Javier Alejandro Jorge

---

### Integrantes y Contacto

| Nombre y Apellido | Correo Electrónico |
| :--- | :--- |
| **Sergio Andres Fernandez Segovia** | _sergio.fernandez.segovia@mi.unc.edu.ar_ |
| **Enzo Leonel Laura Surco** | _enzo.laura.surco@mi.unc.edu.ar_ |
| **Saqib Daniel Mohammad Cabrejos** | _saqib.mohammad@mi.unc.edu.ar_ |


## Introduccion

Este trabajo practico se basa en poner en práctica los conocimientos sobre performance y rendimiento de los computadores. El trabajo consta de dos partes, la primera es utilizar benchmarks de terceros para tomar decisiones de hardware y la segunda consiste en utilizar herramientas para medir la performance de nuestro código y como extra jecutaremos un programa en un ESP32 y observaremos que pasa si variamos su frecuencia.


## Parte 1

Para la evaluacion del rendimiento de hardware se puede realizar pruebas de programa o benchmark lo cual consiste en  medir el tiempo que los diversos computadores tardan en ejecutar los programas que realmente el usuario va a utilizar posteriormente siendo este el mejor rendimiento para este usuario.
Y dependiendo de las tareas diarias podemos usar ciertos benchmarks especificos los cuales serian los mas representativos. 

## Benchmarks para Tareas Diarias:

| Tarea diaria                              | Benchmark recomendado            |
|------------------------------------------|---------------------------------|
| Compilacion de Codigo Fuente              | Phoronix Test Suite: build-linux-kernel              |
| Uso general (navegador, multitarea)      | Geekbench                       |
| Renderizado 3D / diseño                  | Cinebench                       |
| Edición de video / multimedia            | Blender Benchmark               |
| Juegos                                   | 3DMark / benchmarks de GPU      |

## Parte 2

En esta parte del trabajo practico analizaremos el rendimiento de distintos procesadores mas precisamente; Intel Core i5-13600K, AMD Ryzen 9 5900X 12-Core y AMD Ryzen 9 7950X 16-Core. Para ello utilizaremos las metricas proporcionadas por el benchmark:

Phoronix Test Suite: build-linux-kernel / https://openbenchmarking.org/test/pts/build-linux-kernel-1.15.0

|Procesador|Tiempo Promedio(seg)|
|----|----|
|Intel Core i5-13600K|	72 +/- 5|
|AMD Ryzen 9 5900X 12-Core|	76 +/- 8|
|AMD Ryzen 9 7950X 16-Core|50 +/- 6|

### Análisis de Rendimiento

El AMD Ryzen 9 7950X se posiciona como el procesador con rendimiento segun las metricas proporcionadas, eso significa que logran mejor tiempo de compilacion para el kernel de linux, su arquitectura y mayor cantidad de nucleos permiten procesamiento mas eficiente en cargas de trabajo. El Intel Core i5-13600K ofrece tambien un buen rendimiento manteniendose en una posicion intermedia siendo una opcion competitiva con un equilibrio entre costo y rendimiento
El AMD Ryzen 9 7950X se posiciona como el procesador con mejor rendimiento en esta prueba, logrando tiempos de compilación significativamente menores. Su arquitectura y mayor cantidad de núcleos permiten un procesamiento más eficiente en cargas de trabajo altamente paralelizables como la compilación del kernel. Y por ultimo el AMD Ryzen 9 5900X muestra un desempeño inferior para es tarea en comparacion con los otros dos; sigue siendo una opcion viable pero con un tiempo de compilacion mas alto indicando que no aprovecha tan bien sus recursos para esta tarea especifica


### ¿Cual es la aceleración cuando usamos un AMD Ryzen 9 7950X 16-Core?

Cuando hablamos de aceleracion tambien podemos hablar de speedup lo cual viene siendo, cuantas veces mas rapido es un procesador comparado con otro y para ello:


$Speedup = Aceleracion = \frac{EX_{cpu \ original}}{EX_{cpu \ mejorado}} = \frac{76}{50} = 1.52$

Esto significa que el AMD Ryzen 9 7950X 16-Core es $1.52$ veces mas rapido que el AMD Ryzen 9 5900X 12-Core en tareas de compilacion del kernel de Linux. Esta mejora se debe principalmente a la mayor cantidad de nucleos y a la arquitectura mas moderna del procesador.



## Parte 3

Para el desarrollo de esta parte del trabajo practico se hizo uso del modulo **ESP32** que sigue una architectura de 32 bits, y que en conjunto con el entorno de desarrollo **platform.io** se logro cargar el programa y evaluar la performance en dicho modulo.

Para realizar tal evaluacion se realizo un programa que evaluara alrededor de 10s la ejecucion del mismo, con una **frec_clk = 240MHz** (frecuencia por defecto), que luego se iría variando a **160MHz** y **80MHz** para responder la duda de **¿Qué sucede con el tiempo del programa al duplicar (variar) la frecuencia?**

Para lograr la evaluacion parcial del programa se utilizaron 2 bucles for, en el cual un bucle es utilizado para evaluar **operaciones de sumas de enteros** y el otro bucle es utilizado para evaluar **operaciones de sumas de flotantes**.

### Salida del programa en ejecución (variando la frec_clk de la ESP32)
![image](https://hackmd.io/_uploads/HkvIGkR9-x.png)

### Tabla de medidas de desempeño evaluadas:

| Medida de Desempeño | Fórmula de Cálculo / Criterio | 240 MHz (Base) | 160 MHz | 80 MHz |
| :--- | :--- | :--- | :--- | :--- |
| **($T_{prog}$) Latencia** | Dato extraído por consola | **E:** 10.5922s <br> **F:** 11.7268s | **E:** 15.9387s <br> **F:** 17.6401s | **E:** 32.6070s <br> **F:** 36.0985s |
| **($\eta$) Rendimiento** | $\eta = \frac{1}{T_{prog}}$ | **E:** 0.094 exe/s <br> **F:** 0.085 exe/s | **E:** 0.062 exe/s <br> **F:** 0.056 exe/s | **E:** 0.031 exe/s <br> **F:** 0.028 exe/s |
| **($S$) SpeedUp** | $S = \frac{T_{240MHz}}{T_{Actual}}$ | **E:** 1.0 (100%) <br> **F:** 1.0 (100%) | **E:** 0.66 (66%) <br> **F:** 0.66 (66%) | **E:** 0.33 (33%) <br> **F:** 0.33 (33%) |
| **Ciclos Consumidos** | Dato extraído por consola | **E:** 2542119509 <br> **F:** 2814423583 | **E:** 2550190615 <br> **F:** 2822415554 | **E:** 2608558770 <br> **F:** 2887876710 |
| **Ciclos por Iteración** | $CPI_{bucle} = \frac{Ciclos Consumidos}{Ciclos Totales}$ | **E:** 28.24 ciclos <br> **F:** 31.27 ciclos | **E:** 28.33 ciclos <br> **F:** 31.36 ciclos | **E:** 28.98 ciclos <br> **F:** 32.09 ciclos |

**Nota Aclaratoria: Sobre "CiclosTotales" en el cálculo del CPI**

Para el cálculo de los Ciclos por Iteración ($CPI_{bucle}$) se utilizó el valor fijo de 90.000.000 en el denominador debido a dos razones arquitectónicas y de diseño de la prueba:

1. **Diseño del Benchmark y Watchdog Timer:** Para lograr un tiempo de latencia cercano a los 10 segundos requeridos para la medición empírica sin que el *Watchdog Timer (WDT)* de FreeRTOS detectara un bloqueo en la CPU y reiniciara la placa, el algoritmo se dividió en 9 bloques secuenciales de 10.000.000 iteraciones cada uno (totalizando 90 millones de ciclos lógicos en C++).
2. **Limitación de Abstracción en C++:** Al compilar desde un lenguaje de alto nivel forzando la anulación de optimizaciones (`#pragma GCC optimize ("O0")`), una simple línea aritmética como `suma = suma + 1` se traduce en múltiples microinstrucciones en ensamblador Xtensa (operaciones de carga desde RAM, procesamiento en la ALU/FPU, almacenamiento, incremento de contador, comparación y salto). Al no disponer del conteo exacto de microinstrucciones de máquina post-compilación, resulta inviable calcular el CPI (Ciclos por Instrucción) clásico. En su lugar, se definió la métrica $CPI_{bucle}$, la cual representa el costo total promedio en ciclos de reloj que le toma al silicio completar una iteración del algoritmo.

### ¿Qué sucede con el tiempo del programa al duplicar (variar) la frecuencia?

Al duplicar la frecuencia de reloj del procesador (por ejemplo, escalando de **80 MHz a 160 MHz)**, el tiempo de ejecución del programa **(Latencia)** se reduce de manera casi matemáticamente perfecta a la mitad. Sin embargo, este fenómeno no se explica simplemente por la velocidad de la CPU, sino por el comportamiento de la carga de trabajo a nivel arquitectónico, lo cual se evidencia en las métricas obtenidas:

**1. La constancia del esfuerzo físico (Ciclos de Reloj):**
La revelación más importante de las mediciones empíricas es la fila de Ciclos Totales Consumidos. Ya sea a 80, 160 o 240 MHz, el microcontrolador consumió invariablemente alrededor de **2.560 millones de ciclos** para los enteros y **2.830 millones de ciclos** para los flotantes. Esto demuestra que la cantidad de "trabajo físico" (el número de microinstrucciones que la ALU o la FPU deben resolver) es una constante absoluta impuesta por el **código compilado**, totalmente independiente de la frecuencia a la que opere el sistema.

**2. Relación con la Latencia ($T_{prog}$) y el SpeedUp ($S$):**
Al mantener los ciclos totales como una constante, el tiempo de ejecución queda atado exclusivamente a la duración de cada ciclo (el periodo, dictado por la frecuencia).$$T_{prog} = Ciclos\_Totales \times \frac{1}{f_{CPU}}$$Esto se refleja fielmente en los datos experimentales: en la prueba de enteros, al duplicar la frecuencia base de 80 MHz a 160 MHz, la latencia cayó de 32.6070s a 15.9387s. Al calcular el **SpeedUp** tomando los 240 MHz como el 100% del rendimiento ($1.0$), vemos que a 160 MHz el procesador entrega un 66% del rendimiento máximo, y a 80 MHz cae exactamente al 33%. La escalabilidad es estrictamente lineal.

**3. Conclusión de Arquitectura (El escenario "CPU-Bound"):**
Este nivel de eficiencia perfecta al duplicar o triplicar la frecuencia solo se logra porque el algoritmo evaluado es estrictamente **CPU-Bound** (limitado por el procesador). Al ser operaciones aritméticas puras (sumas en bucle), los datos viven en los registros internos del procesador y en la memoria caché más rápida.

### 🛠️ Instrucciones de Compilación y Reproducción del Entorno

Para garantizar la correcta compilación del proyecto `PerformanceESP32-LauraSurco` y evitar errores de dependencias (como la falsa alerta de que no se detecta la librería `<Arduino.h>`), es estrictamente necesario abrir el entorno de manera aislada. 

PlatformIO requiere que el archivo de configuración `platformio.ini` se encuentre en el directorio raíz del espacio de trabajo (*workspace*) activo en el editor (ej. VS Code). 

**Pasos para ejecutar la prueba en una nueva ESP32:**
1. Clonar o descargar el repositorio completo en su máquina local.
2. En lugar de abrir la carpeta principal del repositorio, abrir **únicamente la subcarpeta del proyecto** (donde reside el archivo `platformio.ini`) en una **nueva ventana separada** de su editor de código.
3. Conectar la placa ESP32 mediante USB.
4. Esperar unos segundos a que PlatformIO inicialice el entorno y descargue el framework de Arduino y las *toolchains* de la arquitectura Xtensa de ser necesario.
5. Ejecutar la tarea de *Upload and Monitor* para compilar, flashear el binario y visualizar las estadísticas de desempeño en la terminal serie.
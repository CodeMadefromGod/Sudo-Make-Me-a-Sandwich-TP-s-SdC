#include <Arduino.h>

// ============================================================================
// CONFIGURACIÓN GLOBAL Y CONSTANTES
// ============================================================================

// --- Pines ---
constexpr uint8_t PIN_LED_INDICADOR = 2;

// --- Parámetros de Carga (Benchmark) ---
constexpr uint32_t CANTIDAD_BLOQUES = 9;                  // Bloques para evitar el Watchdog
constexpr uint32_t ITERACIONES_POR_BLOQUE = 10000000UL;   // Iteraciones por cada bloque
constexpr int INCREMENTO_ENTERO = 1;
constexpr float INCREMENTO_FLOTANTE = 1.0f;

// --- Frecuencias del Sistema ---
constexpr int N_FRECUENCIAS = 3;
constexpr int FRECUENCIA_SISTEMA_MHZ[N_FRECUENCIAS] = {240, 160, 80};

// --- Tiempos y Comunicaciones ---
constexpr uint32_t BAUD_RATE_SERIAL = 115200;
constexpr uint32_t TIEMPO_ESPERA_INICIO_MS = 2000;
constexpr uint32_t TIEMPO_ESPERA_IDLE_MS = 1000;
constexpr uint32_t TIEMPO_RESPIRA_WATCHDOG_MS = 1;
constexpr uint32_t TIEMPO_ESTABILIZA_UART_MS = 20;
constexpr uint32_t TIEMPO_VISUALIZACION_RESULTS_MS = 2000;
constexpr float FACTOR_MICROS_A_SEGUNDOS = 1000000.0f;

// ============================================================================

void testEnteros();
void testFlotantes();
void ejecutarPrueba(int frecuencia_mhz);

void setup() {
  Serial.begin(BAUD_RATE_SERIAL);
  delay(TIEMPO_ESPERA_INICIO_MS); 

  pinMode(PIN_LED_INDICADOR, OUTPUT);
  digitalWrite(PIN_LED_INDICADOR, LOW);

  Serial.printf("\n>>> INICIANDO PRUEBAS DE RENDIMIENTO <<<\n");
  
  for (int i = 0; i < N_FRECUENCIAS; i++) {
    ejecutarPrueba(FRECUENCIA_SISTEMA_MHZ[i]);
  }

  Serial.printf("\n>>> PRUEBA FINALIZADA <<<\n");
}

void loop() {
  delay(TIEMPO_ESPERA_IDLE_MS); // Bucle seguro, no hace nada más.
}

void ejecutarPrueba(int frecuencia_mhz) {
  Serial.flush(); // Asegura que todo el texto anterior se haya enviado antes de cambiar la frecuencia
  setCpuFrequencyMhz(frecuencia_mhz);
  delay(TIEMPO_ESTABILIZA_UART_MS);
  delay(TIEMPO_VISUALIZACION_RESULTS_MS); // Tiempo para visualizar frec_clk antes de cada prueba
  Serial.updateBaudRate(BAUD_RATE_SERIAL);  // Reconfigura el baudrate para mantener la comunicación estable tras el cambio de frecuencia
  Serial.printf("\n>>> FRECUENCIA CONFIGURADA: %d MHz <<<\n", frecuencia_mhz);
  testEnteros();
  testFlotantes();
  Serial.println("");
  Serial.flush(); // Asegura que los resultados se envíen antes de cambiar la frecuencia o finalizar la prueba
}

// Bloqueamos la optimización para forzar el procesamiento real en la ALU/FPU
#pragma GCC push_options
#pragma GCC optimize ("O0")

void testEnteros() {
  volatile int suma_int = 0;
  digitalWrite(PIN_LED_INDICADOR, HIGH); 
  
  uint32_t inicio_ciclos = ESP.getCycleCount();
  uint32_t inicio_tiempo = micros();

  // Bucle exterior: Control de Watchdog
  for (uint32_t b = 0; b < CANTIDAD_BLOQUES; b++) {
    delay(TIEMPO_RESPIRA_WATCHDOG_MS); // Forzamos cambio de contexto
    
    // Bucle interior: Procesamiento matemático puro
    for (uint32_t i = 0; i < ITERACIONES_POR_BLOQUE; i++) {
      suma_int = suma_int + INCREMENTO_ENTERO; 
    }
  }

  uint32_t fin_tiempo = micros();
  uint32_t fin_ciclos = ESP.getCycleCount();

  digitalWrite(PIN_LED_INDICADOR, LOW); 

  float tiempo_segundos = (fin_tiempo - inicio_tiempo) / FACTOR_MICROS_A_SEGUNDOS;
  uint32_t ciclos_totales = fin_ciclos - inicio_ciclos;

  Serial.printf("\nEnteros -> Tiempo: %.4f s | Ciclos: %u\n", tiempo_segundos, ciclos_totales);
  Serial.flush(); // Asegura que los resultados se envíen antes de seguir
}

void testFlotantes() {
  volatile float suma_float = 0.0f;
  digitalWrite(PIN_LED_INDICADOR, LOW); // Indicador para diferenciar de la prueba de enteros
  
  uint32_t inicio_ciclos = ESP.getCycleCount();
  uint32_t inicio_tiempo = micros();

  for (uint32_t b = 0; b < CANTIDAD_BLOQUES; b++) {
    delay(TIEMPO_RESPIRA_WATCHDOG_MS); 
    
    for (uint32_t i = 0; i < ITERACIONES_POR_BLOQUE; i++) {
      suma_float = suma_float + INCREMENTO_FLOTANTE; 
    }
  }

  uint32_t fin_tiempo = micros();
  uint32_t fin_ciclos = ESP.getCycleCount();

  digitalWrite(PIN_LED_INDICADOR, HIGH); 

  float tiempo_segundos = (fin_tiempo - inicio_tiempo) / FACTOR_MICROS_A_SEGUNDOS;
  uint32_t ciclos_totales = fin_ciclos - inicio_ciclos;

  Serial.printf("\nFlotantes -> Tiempo: %.4f s | Ciclos: %u\n", tiempo_segundos, ciclos_totales);
  Serial.flush(); // Asegura que los resultados se envíen antes de seguir
}

#pragma GCC pop_options
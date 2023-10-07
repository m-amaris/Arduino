/* Grupo 8
Alumnos: ESTEBAN y MIGUEL
Multiactuador básico de la práctica 0, compuesto por un actuador de enchufe y un actuador de interruptor.
El actuador de enchufe tiene conectado como carga un led blanco, y para simular su funcionamiento, se enciende y apaga a intevalos regulares.
El actuador de interruptor tiene conectado como carga un led rojo, que es activado por un relé, mediante el cual se activa/desactiva la carga.
*/

/* Definición de constantes COMUNES */
#define SI true
#define NO false
#define ENCENDIDO HIGH
#define APAGADO LOW

/* Definición de constantes y variables del ACTUADOR DE ENCHUFE */
const int pin_carga_actuador_enchufe = 15; // Pin al que esta conectado la carga
const long intervalo_encendido = 2000; // Encendido durante 2 segundos
const long intervalo_apagado = 1000; // Apagado durante 1 segundo
unsigned long tiempo_previo = 0; // Unsigned para variables que guardan tiempo, almacena el tiempo en el que se realizó el último cambio de estado
bool estado_carga_actuador_enchufe = APAGADO; // Estado inicial de la carga (apagado)

/* Definición de constantes y variables del ACTUADOR DE INTERRUPTOR*/
const int pin_rele_actuador_interruptor = 13; // Pin al que esta conectado la carga
const int pin_pulsador_interruptor = 5; // Pin al que está conectado el pulsador de interruptor
bool estado_anterior_pulsador = ENCENDIDO; // Estado anterior del pulsador (inicializado a alto)
bool estado_carga_actuador_interruptor = APAGADO; // Estado inicial de la carga (apagado)
bool pulsador_estaba_presionado = NO; // Variable para rastrear si el pulsador está presionado

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Arrancando");
  setup_actuador_enchufe();
  setup_actuador_interruptor();
  Serial.println("Inicialización del dispositivo terminada.");
}

void setup_actuador_enchufe() {
  Serial.print("Inicializando el actuador de enchufe ");
  pinMode(pin_carga_actuador_enchufe, OUTPUT);
  Serial.println("hecho.");
}

void setup_actuador_interruptor() {
  Serial.println("Inicializando el actuador de interruptor ");
  pinMode(pin_rele_actuador_interruptor, OUTPUT);
  pinMode(pin_pulsador_interruptor, INPUT_PULLUP); // Utiliza resistencia pull-up interna
  Serial.println("hecho.");
}

void loop() {
  // Código común a ambos actuadores
  loop_actuador_interruptor();
  loop_actuador_enchufe();
}

void loop_actuador_enchufe() {
  unsigned long tiempo_actual = millis(); // Almacena cuanto tiempo ha pasado desde el último estado  

  if (estado_carga_actuador_enchufe == APAGADO && tiempo_actual - tiempo_previo >= intervalo_apagado) {
    estado_carga_actuador_enchufe = ENCENDIDO; // Cambia el estado del actuador
    tiempo_previo = tiempo_actual; // Guarda el tiempo actual
    Serial.println("Enciendo");
    digitalWrite(pin_carga_actuador_enchufe, estado_carga_actuador_enchufe);

  } else if (estado_carga_actuador_enchufe == ENCENDIDO && tiempo_actual - tiempo_previo >= intervalo_encendido) {
    estado_carga_actuador_enchufe = APAGADO; // Cambia el estado del actuador
    tiempo_previo = tiempo_actual; // Guarda el tiempo actual
    Serial.println("Apago\n");
    digitalWrite(pin_carga_actuador_enchufe, estado_carga_actuador_enchufe);
  }
}

void loop_actuador_interruptor() {
  int estado_pulsador = digitalRead(pin_pulsador_interruptor); // Almacena en una variable el si el pulsador está pulsado o no

  // Si el pulsador se presiona y no estaba presionado previamente, enciende el actuador
  if (estado_pulsador == APAGADO && estado_anterior_pulsador == ENCENDIDO && !pulsador_estaba_presionado) {
    estado_carga_actuador_interruptor = !estado_carga_actuador_interruptor; // Cambia el estado del actuador
    pulsador_estaba_presionado = SI; // Marca que el pulsador está presionado
    digitalWrite(pin_rele_actuador_interruptor, estado_carga_actuador_interruptor); // Aplica el cambio al actuador
    Serial.println(estado_carga_actuador_interruptor == ENCENDIDO ? "LED encendido" : "LED apagado");
  }

  // Si el pulsador se suelta, marca que el pulsador ya no está presionado
  if (estado_pulsador == ENCENDIDO && pulsador_estaba_presionado) {
    pulsador_estaba_presionado = NO;
  }

  estado_anterior_pulsador = estado_pulsador; // Guarda el estado actual del pulsador para la próxima comparación
}
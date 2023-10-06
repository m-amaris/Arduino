/* Grupo X
Alumnos: AAAA y BBBBB
Multiactuador básico de la práctica 0, compuesto por un actuador de enchufe y un actuador de interruptor.
El actuador de enchufe tiene conectado como carga un led blanco, y para simular su funcionamiento, se enciende y apaga a intevalos regulares.
El actuador de interruptor tiene conectado como carga un led rojo, que es activado por un relé. Mediante un se activa/desactiva la carga.
*/
//#include XXXXX

/* Definición de constantes COMUNES */
#define SI true
#define NO false
#define ENCENDIDO HIGH
#define APAGADO LOW

/* Definición de constantes y variables del ACTUADOR DE ENCHUFE */
const int pin_carga_actuador_enchufe = 15;
#define APAGADO_ENCHUFE LOW
bool estado_carga_actuador_enchufe = APAGADO;
const long intervalEncendido = 2000;  // LED encendido durante 2 segundos
const long intervalApagado = 1000;    // LED apagado durante 1 segundo
//const long intervalo = 1000;  // intervalo en el que parpadea(milisegundos)
unsigned long previousMillis = 0;  // unsigned para variables que guardan tiempo

/* Definición de constantes y variables del ACTUADOR DE INTERRUPTOR*/
const int carga_actuador_enchufe = 13;        // Nº del pin que se quiere usar como salida
const int pinPulsadorInterruptor = 5;         // Pin al que está conectado el pulsador de interruptor

int estadoLed = APAGADO;                      // Estado inicial del LED (apagado)
int estadoPulsadorAnterior = ENCENDIDO;       // Estado anterior del pulsador (inicializado a alto)
bool pulsadorEstaPresionado = NO;             // Variable para rastrear si el pulsador está presionado


//const int pin_carga_actuador_interruptor = 12;
//const int pin_rele_actuador_interruptor = 13;
//const int pin_pulsador_interruptor = 15;
//bool estado_carga_actuador_interruptor = APAGADO;

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
  pinMode(carga_actuador_enchufe, OUTPUT);
  pinMode(pinPulsadorInterruptor, INPUT_PULLUP); // Utiliza resistencia pull-up interna
  Serial.println("hecho.");
}

void loop() {
  // Código común a ambos actuadores 1oop_actuador_enchufe();
  loop_actuador_interruptor();
  loop_actuador_enchufe();
}

void loop_actuador_enchufe() {
  unsigned long currentMillis = millis();

  if (estado_carga_actuador_enchufe == APAGADO && currentMillis - previousMillis >= intervalApagado) {
    estado_carga_actuador_enchufe = ENCENDIDO;
    previousMillis = currentMillis;  // Guarda el tiempo actual
    Serial.println("Enciendo");
    digitalWrite(pin_carga_actuador_enchufe, estado_carga_actuador_enchufe);

  } else if (estado_carga_actuador_enchufe == ENCENDIDO && currentMillis - previousMillis >= intervalEncendido) {
    estado_carga_actuador_enchufe = APAGADO;
    previousMillis = currentMillis;  // Guarda el tiempo actual
    Serial.println("Apago\n");
    digitalWrite(pin_carga_actuador_enchufe, estado_carga_actuador_enchufe);
  }
}


void loop_actuador_interruptor() {
  int estadoDelPulsador = digitalRead(pinPulsadorInterruptor); //Almacena en una variable el si el pulsador esta pulsado o no
  
  // Si el pulsador se presiona y no estaba presionado previamente, enciende el actuador
  if (estadoDelPulsador == APAGADO && estadoPulsadorAnterior == ENCENDIDO && !pulsadorEstaPresionado) {
    estadoLed = !estadoLed;                          // Cambia el estado del actuador
    pulsadorEstaPresionado = SI;                     // Marca que el pulsador está presionado
    digitalWrite(carga_actuador_enchufe, estadoLed); // Aplica el cambio al actuador
    Serial.println(estadoLed == ENCENDIDO ? "LED encendido" : "LED apagado");
  }
  
  // Si el pulsador se suelta, marca que el pulsador ya no está presionado
  if (estadoDelPulsador == ENCENDIDO && pulsadorEstaPresionado) {
    pulsadorEstaPresionado = NO;
  }

  estadoPulsadorAnterior = estadoDelPulsador;        // Guarda el estado actual del pulsador para la próxima comparación
}
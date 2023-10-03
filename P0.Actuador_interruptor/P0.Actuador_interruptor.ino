// Actuador de Interruptor v2

// Constantes para el actuador de interruptor
const bool ENCENDIDO = HIGH;
const bool APAGADO = LOW;
const int pin_carga_actuador_interruptor = 13;	// Pin al que está conectado el LED
const int pinPulsadorInterruptor = 2;   // Pin al que está conectado el pulsador de interruptor
const int tiempoDebounce = 50;          // Tiempo de debounce en milisegundos
const int estadoPulsado = APAGADO;          // Estado del pulsador cuando está presionado

// Variables para el actuador de interruptor
int estadoLed = APAGADO;          // Estado actual del LED (apagado)
int ultimoEstadoPulsador = ENCENDIDO; // Estado del pulsador en la última lectura
unsigned long ultimoTiempoDebounce = 0;  // Último tiempo en que se leyó el pulsador

void setup() {
  pinMode(pin_carga_actuador_interruptor, OUTPUT);
  pinMode(pinPulsadorInterruptor, INPUT_PULLUP); // Habilitar la resistencia pull-up interna
  Serial.begin(115200); // Iniciar la comunicación serial
}

void loop() {
  int lecturaPulsador = digitalRead(pinPulsadorInterruptor);
  
  // Verificar el estado del pulsador con debounce
  if (lecturaPulsador != ultimoEstadoPulsador) {
    ultimoTiempoDebounce = millis();
  }

  if ((millis() - ultimoTiempoDebounce) > tiempoDebounce) {
    if (lecturaPulsador != estadoPulsado) {
      if (estadoLed == APAGADO) {
        estadoLed = ENCENDIDO;
        Serial.println("LED encendido");
      } else {
        estadoLed = APAGADO;
        Serial.println("LED apagado");
      }
      digitalWrite(pin_carga_actuador_interruptor, estadoLed);
    }
  }
  
  // Actualizar el estado anterior del pulsador
  ultimoEstadoPulsador = lecturaPulsador;
}

const int pinLed = 13;                        // Pin al que está conectado el LED
const int pinPulsadorInterruptor = 5;        // Pin al que está conectado el pulsador de interruptor

int estadoLed = LOW;                         // Estado inicial del LED (apagado)
int estadoPulsadorAnterior = HIGH;           // Estado anterior del pulsador (inicializado a alto)
bool pulsadorPresionado = false;             // Variable para rastrear si el pulsador está presionado

void setup() {
  pinMode(pinLed, OUTPUT);
  pinMode(pinPulsadorInterruptor, INPUT_PULLUP); // Utiliza resistencia pull-up interna
  Serial.begin(115200);  // Inicializa la comunicación serial para la consola
}

void loop() {
  int estadoPulsador = digitalRead(pinPulsadorInterruptor);
  
  // Si el pulsador se presiona y no estaba presionado previamente, enciende el LED
  if (estadoPulsador == LOW && estadoPulsadorAnterior == HIGH && !pulsadorPresionado) {
    estadoLed = !estadoLed; // Cambia el estado del LED
    pulsadorPresionado = true; // Marca que el pulsador está presionado
    digitalWrite(pinLed, estadoLed); // Aplica el cambio al LED
    Serial.println(estadoLed == HIGH ? "LED encendido" : "LED apagado");
  }
  
  // Si el pulsador se suelta, marca que el pulsador ya no está presionado
  if (estadoPulsador == HIGH && pulsadorPresionado) {
    pulsadorPresionado = false;
  }

  estadoPulsadorAnterior = estadoPulsador; // Guarda el estado actual del pulsador para la próxima comparación
}

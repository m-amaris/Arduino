const bool ENCENDIDO = HIGH;
const bool APAGADO = LOW;
const bool SI = true;
const bool NO = false;
const int carga_actuador_enchufe = 13;        // Nº del pin que se quiere usar como salida
const int pinPulsadorInterruptor = 5;         // Pin al que está conectado el pulsador de interruptor

int estadoLed = APAGADO;                      // Estado inicial del LED (apagado)
int estadoPulsadorAnterior = ENCENDIDO;       // Estado anterior del pulsador (inicializado a alto)
bool pulsadorEstaPresionado = NO;             // Variable para rastrear si el pulsador está presionado

void setup() {
  pinMode(carga_actuador_enchufe, OUTPUT);
  pinMode(pinPulsadorInterruptor, INPUT_PULLUP); // Utiliza resistencia pull-up interna
  Serial.begin(115200);  // Inicializa la comunicación serie para la consola
}

void loop() {
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

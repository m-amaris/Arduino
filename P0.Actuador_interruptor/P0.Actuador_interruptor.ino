const int ledPin = 13;    // Pin al que está conectado el LED
const int buttonPin = 2;  // Pin al que está conectado el pulsador
int ledState = LOW;       // Estado inicial del LED (apagado)
int lastButtonState = LOW; // Estado del pulsador en la última lectura
int buttonState;          // Estado actual del pulsador
unsigned long lastDebounceTime = 0;  // Último tiempo en que se leyó el pulsador
unsigned long debounceDelay = 50;    // Tiempo de debounce (evita lecturas falsas)

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  // Lee el estado del pulsador y aplica debounce
  int reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      // Si el estado del pulsador cambió, realiza la acción
      if (buttonState == LOW) {
        // El pulsador ha sido presion
        // El pulsador ha sido presionado
        if (ledState == LOW) {
          // Si el LED está apagado, enciéndelo
          ledState = HIGH;
        } else {
          // Si el LED está encendido, apágalo
          ledState = LOW;
        }
        digitalWrite(ledPin, ledState);
      }
    }
  }

  // Guarda el estado actual del pulsador para la próxima comparación
  lastButtonState = reading;
}

const bool ENCENDIDO = HIGH;
const bool APAGADO = LOW;
const long interval = 1000;  // interval at which to blink (milliseconds)
const int carga_actuador_enchufe = 8; // Nº del pin que se quiere usar como salida

int estado_carga_actuador_enchufe = APAGADO;  
unsigned long previousMillis = 0;  // unsigned para variables que guardan tiempo
int vueltas;

void setup()
{
	pinMode(carga_actuador_enchufe, OUTPUT);
	Serial.begin(115200);
	vueltas=0;
}

void loop()
{
unsigned long currentMillis = millis();

if (currentMillis - previousMillis >= interval) {
    Serial.println("Llevo "+String(vueltas)+" vuelta(s)");
    previousMillis = currentMillis; // save the last time you blinked the LED

    if (estado_carga_actuador_enchufe == APAGADO) {
      estado_carga_actuador_enchufe = ENCENDIDO;
      
      Serial.println("Enciendo ");
      vueltas++;
    } else {
      estado_carga_actuador_enchufe = APAGADO;
      Serial.println("Apago\n");
    }
    digitalWrite(carga_actuador_enchufe, estado_carga_actuador_enchufe);
	  
  }
}
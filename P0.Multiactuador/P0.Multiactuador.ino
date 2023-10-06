/* Grupo X
Alumnos: AAAA y BBBBB
Multiactuador básico de la práctica 0, compuesto por un actuador de enchufe y un actuador de interruptor.
El actuador de enchufe tiene conectado como carga un led blanco, y para simular su funcionamiento, se enciende y apaga a intevalos regulares.
El actuador de interruptor tiene conectado como carga un led rojo, que es activado por un relé. Mediante un se activa/desactiva la carga.
*/
#include XXXXX
/* Definición de constantes COMUNES */ #define ENCENDER HIGH
#define ENCENDIDO true
/* Definición de constantes y variables del ACTUADOR DE ENCHUFE */
const int pin_carga_actuador_enchufe = 11;
#define APAGADO_ENCHUFE LOW
bool estado_carga_actuador_enchufe=APAGADO;
Hogar Digital
* Definición de constantes y variables del ACTUADOR DE INTERRUPTOR*/
const int pin_carga_actuador_interruptor = 12;
const int pin_rele actuador_interruptor = 13;
const int pin_pulsador_interruptor = 15;
bool estado_carga_actuador_enchufe=APAGADO;
void setup() {
Serial. begin(115200);
Serial.printIn("");
Serial. printIn("Arrancando");
setup_actuador_enchufe();
setup_actuador_interruptor();
Serial.printIn("Inicialización del dispositivo terminada.");
}
void setup_actuador_enchufe() {
Serial.print("Inicializando el actuador de enchufe ");
Serial. printIn("hecho.");
｝
void setup_actuador_interruptor() {
serial.print("Inicializando el actuador de interruptor ");
Serial.printIn("hecho.");
}
void 100p() {
Il Código común a ambos actuadores
1oop_actuador_enchufe();
1oop_actuador_ interruptor();
}
void loop_actuador_enchufe() {}
void Loop_actuador_interruptor() {}
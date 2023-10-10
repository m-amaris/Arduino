/* Grupo 8
Alumnos: ESTEBAN y MIGUEL
Multiactuador básico de la práctica 0, compuesto por un actuador de enchufe y un actuador de interruptor.
El actuador de enchufe tiene conectado como carga un led blanco, y para simular su funcionamiento, se enciende y apaga a intevalos regulares.
El actuador de interruptor tiene conectado como carga un led rojo, que es activado por un relé, mediante el cual se activa/desactiva la carga.
*/

#include <ESP8266WiFi.h>       // biblioteca wifi de esp8266
#include <ESP8266WebServer.h>  // Servidor web

// Definición de constantes
#define ENCENDER HIGH
#define APAGAR LOW
// Relé (Interruptor)
#define PINPULSADOR_INTERRUPTOR 0  // Pin al que está conectado el pulsador de interruptor
#define PINRELE_INTERRUPTOR 13     // Pin donde irá conectado el relé
#define PINCARGA_ENCHUFE 15
#define PINCARGA_PERSIANA_BAJAR 5
#define PINCARGA_PERSIANA_SUBIR 4

//Enchufe
bool estado_carga_actuador_enchufe = APAGAR;
// Interruptor
bool estado_anterior_pulsador = ENCENDER;         // Estado anterior del pulsador (inicializado a alto)
bool estado_carga_actuador_interruptor = APAGAR;  // Estado inicial de la carga (apagado)
bool pulsador_estaba_presionado = false;          // Variable para rastrear si el pulsador está presionado
//Persiana
byte estado_carga_actuador_persinana = 0;
// Parámetros de la red wifi
const char* ssid = "LABDTE";
const char* password = "envev1d0s";
//IPAddress wifiIP(10, 49, 33, 81);   // IP fija 10.49.33.81
IPAddress wifiIP(192, 168, 137, 252);  // IP fija 10.49.33.81
IPAddress wifiNET(255, 254, 0, 0);     // Máscara de red (/15)
IPAddress wifiON(10, 48, 0, 1);        // Puerta de enlace

// Objecto servidor
ESP8266WebServer servidorWeb(80);

void setup_actuador_enchufe() {
  pinMode(PINCARGA_ENCHUFE, OUTPUT);       // Configurar pin como salida
  digitalWrite(PINCARGA_ENCHUFE, APAGAR);  // Apagar pin de carga de enchufe
}

void setup_actuador_interruptor() {
  pinMode(PINRELE_INTERRUPTOR, OUTPUT);       // Configurar pin como salida
  digitalWrite(PINRELE_INTERRUPTOR, APAGAR);  // Desactivar pin de carga (relé) del interruptor
}

void setup_actuador_persiana() {
  pinMode(PINCARGA_PERSIANA_BAJAR, OUTPUT);       // Configurar pin como salida
  pinMode(PINCARGA_PERSIANA_SUBIR, OUTPUT);       // Configurar pin como salida
  digitalWrite(PINCARGA_PERSIANA_SUBIR, APAGAR);  // Apagar pin de carga de subida
  digitalWrite(PINCARGA_PERSIANA_BAJAR, APAGAR);  // Apagar pin de carga de bajada
}

void setup(void) {
  setup_actuador_enchufe();
  setup_actuador_interruptor();
  setup_actuador_persiana();
  Serial.begin(115200); // Establecer velocidad consola serie
  conectar_WIFI();   //Conexion WIFI
  configura_ServidorWEB(); // Configuración del servidor web:
  servidorWeb.begin(); // Arrancar servidor web
  // PENDIENTE DE EDICION
  Serial.println("Servidor web arrancado");
  Serial.println("Listo. Conectarse a un navegador y usar estas URLs:");
  Serial.print("Para activar: ");
  Serial.print(WiFi.localIP());
  Serial.println("/activar");
  Serial.print("Para desactivar: ");
  Serial.print(WiFi.localIP());
  Serial.println("/desactivar");
}

void loop(void) {
  servidorWeb.handleClient(); // Consultar si se ha recibido una petición al servidor web
  loop_actuador_interruptor();
  loop_actuador_persiana();
  loop_pin_handler();
}

/*
*  Función encargada de imprimir los valores de los pines 
*  una vez son modificados via fisico o HTTP
*/
void loop_pin_handler() {
  digitalWrite(PINCARGA_ENCHUFE, estado_carga_actuador_enchufe);         // Aplica el cambio al actuador del enchufe
  digitalWrite(PINRELE_INTERRUPTOR, estado_carga_actuador_interruptor);  // Aplica el cambio al actuador del interruptor
}

void configura_ServidorWEB() {
  servidorWeb.on("/", manejadorRaiz);
  servidorWeb.on("/enchufe/encender", manejadorEncenderEnchufe);
  servidorWeb.on("/enchufe/apagar", manejadorApagarEnchufe);
  servidorWeb.on("/enchufe/estado", manejadorEstadoEnchufe);
  servidorWeb.on("/interruptor/encender", manejadorEncenderInterruptor);
  servidorWeb.on("/interruptor/apagar", manejadorApagarInterruptor);
  servidorWeb.on("/interruptor/estado", manejadorEstadoInterruptor);
  servidorWeb.on("/persiana/subir", manejadorSubirPersiana);
  servidorWeb.on("/persiana/bajar", manejadorBajarPersiana);
  servidorWeb.on("/persiana/parar", manejadorPararPersiana);
  servidorWeb.onNotFound(paginaNoEncontrada);
}

void conectar_WIFI() {
  delay(10);  // Da tiempo a que se inicialice el hardware de la Wifi
  WiFi.mode(WIFI_STA); // Establecer configuración wifi para no usar DHCP
  WiFi.config(wifiIP, wifiON, wifiNET); //Para no usar DHCP hay que definir la red.
  WiFi.begin(ssid, password);
  Serial.printf("Conectándo a la wifi con SSID %s y clave %s", ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conectado a WiFi!");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

/** FUNCIONES ASOCIADAS AL ENCHUFE **/
void manejadorEstadoEnchufe() {
  if (estado_carga_actuador_enchufe) {
    servidorWeb.send(200, "text/plain", "encendido");
  } else {
    servidorWeb.send(200, "text/plain", "apagado");
  }
}

void manejadorApagarEnchufe() {
  estado_carga_actuador_enchufe = APAGAR;
  servidorWeb.send(200, "text/plain", "OK, actuador de enchufe apagado");
}

void manejadorEncenderEnchufe() {
  estado_carga_actuador_enchufe = ENCENDER;
  servidorWeb.send(200, "text/plain", "OK, actuador de enchufe encendido");
}

/** FUNCIONES ASOCIADAS AL INTERRUPTOR **/
void manejadorEstadoInterruptor() {
  if (estado_carga_actuador_interruptor) {
    servidorWeb.send(200, "text/plain", "encendido");
  } else {
    servidorWeb.send(200, "text/plain", "apagado");
  }
}

void manejadorApagarInterruptor() {
  estado_carga_actuador_interruptor = APAGAR;

  servidorWeb.send(200, "text/plain", "OK, actuador de interruptor apagado");
  Serial.println("Relé desactivado");
}

void manejadorEncenderInterruptor() {
  estado_carga_actuador_interruptor = ENCENDER;

  servidorWeb.send(200, "text/plain", "OK, actuador de interruptor encendido");
  Serial.println("Relé activado");
}

void loop_actuador_interruptor() {
  int estado_pulsador = digitalRead(PINPULSADOR_INTERRUPTOR);  // Almacena en una variable el si el pulsador está pulsado o no

  // Si el pulsador se presiona y no estaba presionado previamente, enciende el actuador
  if (estado_pulsador == APAGAR && estado_anterior_pulsador == ENCENDER && !pulsador_estaba_presionado) {
    estado_carga_actuador_interruptor = !estado_carga_actuador_interruptor;  // Cambia el estado del actuador
    pulsador_estaba_presionado = true;                                       // Marca que el pulsador está presionado

    Serial.println(estado_carga_actuador_interruptor == ENCENDER ? "LED encendido" : "LED apagado");
  }

  // Si el pulsador se suelta, marca que el pulsador ya no está presionado
  if (estado_pulsador == ENCENDER && pulsador_estaba_presionado) {
    pulsador_estaba_presionado = false;
  }

  estado_anterior_pulsador = estado_pulsador;  // Guarda el estado actual del pulsador para la próxima comparación
}

/** FUNCIONES ASOCIADAS A LA PERSIANA **/
void manejadorSubirPersiana() {
  estado_carga_actuador_persinana = 1;
  servidorWeb.send(200, "text/plain", "OK, actuador de persiana subiendo");
}

void manejadorBajarPersiana() {
  estado_carga_actuador_persinana = 2;
  servidorWeb.send(200, "text/plain", "OK, actuador de persiana bajando");
}

void manejadorPararPersiana() {
  estado_carga_actuador_persinana = 0;
  servidorWeb.send(200, "text/plain", "OK, actuador de persiana parado");
}

void loop_actuador_persiana() {
  if (estado_carga_actuador_persinana == 1) {
    digitalWrite(PINCARGA_PERSIANA_SUBIR, ENCENDER);  // Aplica el cambio al actuador del interruptor
    digitalWrite(PINCARGA_PERSIANA_BAJAR, APAGAR);    // Aplica el cambio al actuador del enchufe
  } else if (estado_carga_actuador_persinana == 2) {
    digitalWrite(PINCARGA_PERSIANA_SUBIR, APAGAR);    // Aplica el cambio al actuador del interruptor
    digitalWrite(PINCARGA_PERSIANA_BAJAR, ENCENDER);  // Aplica el cambio al actuador del enchufe
  } else {
    digitalWrite(PINCARGA_PERSIANA_SUBIR, APAGAR);  // Aplica el cambio al actuador del interruptor
    digitalWrite(PINCARGA_PERSIANA_BAJAR, APAGAR);  // Aplica el cambio al actuador del enchufe
  }
}

// Página inicial con el menú de opciones
void manejadorRaiz() {
  String mensaje;
  mensaje = "<!DOCTYPE HTML>\r\n<html>\r\n";
  mensaje += "<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";
  mensaje += "<title>Actuador de enchufe</title></head>\r\n";
  mensaje += "<body>";
  mensaje += "<h1>Multiactuador</h1>";
  mensaje += "<p>Actuador de Enchufe:</p>";
  mensaje += "Pulsa para <a href=/enchufe/encender>Encender</a><br>";
  mensaje += "Pulsa para <a href=/enchufe/apagar>Apagar</a><br>";
  mensaje += "Pulsa para <a href=/enchufe/estado>Consulta de Estado</a>";
  mensaje += "<p>Actuador de Interruptor:</p>";
  mensaje += "Pulsa para <a href=/interruptor/encender>Encender</a><br>";
  mensaje += "Pulsa para <a href=/interruptor/apagar>Apagar</a><br>";
  mensaje += "Pulsa para <a href=/interruptor/estado>Consulta de Estado</a>";
  mensaje += "<p>Actuador de Persiana:</p>";
  mensaje += "Pulsa para <a href=/persiana/subir>Subir</a><br>";
  mensaje += "Pulsa para <a href=/persiana/bajar>Bajar</a><br>";
  mensaje += "Pulsa para <a href=/persiana/parar>Parar</a>";
  mensaje += "</body>";
  mensaje += "</html>\n";
  servidorWeb.send(200, "text/html; charset=UTF-8", mensaje);
}

void paginaNoEncontrada() {
  String mensaje = "Página no encontrada\n\n";
  mensaje += "URI: ";
  mensaje += servidorWeb.uri();
  mensaje += "\nMetodo: ";
  mensaje += (servidorWeb.method() == HTTP_GET) ? "GET" : "POST";
  mensaje += "\nArgumentos: ";
  mensaje += servidorWeb.args();
  mensaje += "\n";
  for (uint8_t i = 0; i < servidorWeb.args(); i++) {
    mensaje += " " + servidorWeb.argName(i) + ": " + servidorWeb.arg(i) + "\n";
  }
  servidorWeb.send(404, "text/plain", mensaje);
}
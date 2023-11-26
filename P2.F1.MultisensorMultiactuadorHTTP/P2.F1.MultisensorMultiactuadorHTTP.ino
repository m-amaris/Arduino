/* Grupo 8
Alumnos: ESTEBAN y MIGUEL
Multiactuador básico de la práctica 0, compuesto por un actuador de enchufe y un actuador de interruptor.
El actuador de enchufe tiene conectado como carga un led blanco, y para simular su funcionamiento, se enciende y apaga a intevalos regulares.
El actuador de interruptor tiene conectado como carga un led rojo, que es activado por un relé, mediante el cual se activa/desactiva la carga.
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>           // Biblioteca para el sensor DHT11
#include <ESP8266HTTPClient.h>

// Definición de constantes
const int ESTADO_APAGADO = LOW;
const int ESTADO_ENCENDIDO = HIGH;
const int PULSADOR_ESTADO_APAGADO = HIGH; // PullUp
const int PULSADOR_ESTADO_ENCENDIDO = LOW; // PullUp

// Pines de actuadores
const int PIN_ENCHUFE = 15;
const int PIN_INTERRUPTOR = 13;
const int PIN_PERSIANA_SUBIR = 5;
const int PIN_PERSIANA_BAJAR = 4;
const int PIN_PULSADOR_INTERRUPTOR = 0;

#define MSG_BUFFER_SIZE (50)
#define TIEMPO_ENTRE_MENSAJES (5000)
#define NUMERO_REINTENTOS (3)
#define TIEMPO_RECONEXION (5000)

// Variables de estado
bool estadoEnchufe = ESTADO_APAGADO;
bool estadoInterruptor = ESTADO_APAGADO;
bool estadoPrevioPulsador = ESTADO_APAGADO;
int estadoPersiana = 0;  // 0 - Parar, 1 - Subir, 2 - Bajar

// Otras variables
bool pulsadorEstabaPresionado = false;

// Variables de WIFI
const char* ssid = "iPhone de Miguel";
const char* password = "envev1d0s";
IPAddress wifiIP(192,168,1,210);  // IP que se asignará al microcontrolador (debe ser una IP sin uso en la red)
IPAddress wifiNET (255,255,255,0); // Máscara de Red
IPAddress wifiON (192,168,1,1); // Dirección IP del router 
ESP8266WebServer server(80);


void setupActuadores() {
  pinMode(PIN_ENCHUFE, OUTPUT);
  pinMode(PIN_INTERRUPTOR, OUTPUT);
  pinMode(PIN_PERSIANA_SUBIR, OUTPUT);
  pinMode(PIN_PERSIANA_BAJAR, OUTPUT);
  pinMode(PIN_PULSADOR_INTERRUPTOR, INPUT_PULLUP); 
}

void setup() {
  setupActuadores();
  Serial.begin(115200);
  conectar_WIFI();
  configurarServidorWeb();
  server.begin();
  Serial.println("Servidor web arrancado");
  Serial.println("Conectarse a un navegador y usar estos URLs:");
  Serial.println("ACTUADOR DE ENCHUFE");
  Serial.print("Para activar el relé: ");
  Serial.print(WiFi.localIP());
  Serial.println("/enchufe/encender");
  Serial.print("Para desactivar el relé: ");
  Serial.print(WiFi.localIP());
  Serial.println("/enchufe/apagar");
  Serial.println("ACTUADOR DE INTERRUPTOR");
  Serial.print("Para activar el relé: ");
  Serial.print(WiFi.localIP());
  Serial.println("/interruptor/encender");
  Serial.print("Para desactivar el relé: ");
  Serial.print(WiFi.localIP());
  Serial.println("/interruptor/apagar");
  Serial.print("Para ver el estado del relé: ");
  Serial.print(WiFi.localIP());
  Serial.println("/interruptor/estado");
  Serial.println("ACTUADOR DE PERSIANA");
  Serial.print("Para subir: ");
  Serial.print(WiFi.localIP());
  Serial.println("/persiana/subir");
  Serial.print("Para bajar: ");
  Serial.print(WiFi.localIP());
  Serial.println("/persiana/bajar");
  Serial.print("Para parar: ");
  Serial.print(WiFi.localIP());
  Serial.println("/persiana/parar");
}

void loop() {
  server.handleClient();
  loopActuadorEnchufe();
  loopActuadorInterruptor();
  loopActuadorPersiana();
}

void conectar_WIFI() {
  delay(10);
  //WiFi.mode(WIFI_STA);
  //WiFi.config(wifiIP, wifiON, wifiNET);
  WiFi.begin(ssid, password);
  Serial.printf("Conectando a la wifi con SSID %s y clave %s", ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conectado a WiFi!");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
}


void configurarServidorWeb() {
  server.on("/", manejadorRaiz);
  server.on("/enchufe/encender", manejadorEncenderEnchufe);
  server.on("/enchufe/apagar", manejadorApagarEnchufe);
  server.on("/enchufe/estado", manejadorEstadoEnchufe);
  server.on("/interruptor/encender", manejadorEncenderInterruptor);
  server.on("/interruptor/apagar", manejadorApagarInterruptor);
  server.on("/interruptor/estado", manejadorEstadoInterruptor);
  server.on("/persiana/subir", manejadorSubirPersiana);
  server.on("/persiana/bajar", manejadorBajarPersiana);
  server.on("/persiana/parar", manejadorPararPersiana);
  server.onNotFound(paginaNoEncontrada);
}

void loopActuadorEnchufe() {
  digitalWrite(PIN_ENCHUFE, estadoEnchufe);
}

void loopActuadorInterruptor() {
  int estadoPulsador = digitalRead(PIN_PULSADOR_INTERRUPTOR); 

  if (estadoPulsador == PULSADOR_ESTADO_ENCENDIDO && estadoPrevioPulsador == PULSADOR_ESTADO_APAGADO && !pulsadorEstabaPresionado) {
    estadoInterruptor = !estadoInterruptor;
    pulsadorEstabaPresionado = true;
    Serial.println(estadoInterruptor == ESTADO_ENCENDIDO ? "Relé activado" : "Relé desactivado");
  } 
  if (estadoPulsador == PULSADOR_ESTADO_APAGADO && pulsadorEstabaPresionado) {
    pulsadorEstabaPresionado = false;
  }

  estadoPrevioPulsador = estadoPulsador;
  digitalWrite(PIN_INTERRUPTOR, estadoInterruptor);
}

void loopActuadorPersiana() {
  if (estadoPersiana == 1) {
    digitalWrite(PIN_PERSIANA_SUBIR, ESTADO_ENCENDIDO);
    digitalWrite(PIN_PERSIANA_BAJAR, ESTADO_APAGADO);
  } else if (estadoPersiana == 2) {
    digitalWrite(PIN_PERSIANA_SUBIR, ESTADO_APAGADO);

    digitalWrite(PIN_PERSIANA_BAJAR, ESTADO_ENCENDIDO);
  } else {
    digitalWrite(PIN_PERSIANA_SUBIR, ESTADO_APAGADO);
    digitalWrite(PIN_PERSIANA_BAJAR, ESTADO_APAGADO);
  }
}

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
  server.send(200, "text/html; charset=UTF-8", mensaje);
}

void manejadorEncenderEnchufe() {
  estadoEnchufe = ESTADO_ENCENDIDO;
  Serial.println("[HTTP] Actuador de enchufe: recibida orden de encender");
  server.send(200, "text/plain", "OK, actuador de enchufe encendido");
}

void manejadorApagarEnchufe() {
  estadoEnchufe = ESTADO_APAGADO;
  Serial.println("[HTTP] Actuador de enchufe: recibida orden de apagar");
  server.send(200, "text/plain", "OK, actuador de enchufe apagado");
}

void manejadorEstadoEnchufe() {
  if (estadoEnchufe) {
    Serial.println("[HTTP] Actuador de enchufe: recibida orden de petición de estado: encendido");
    server.send(200, "text/plain", "encender");
  } else {
    Serial.println("[HTTP] Actuador de enchufe: recibida orden de petición de estado: apagado");
    server.send(200, "text/plain", "apagar");
  }
}

void manejadorEncenderInterruptor() {
  estadoInterruptor = ESTADO_ENCENDIDO;
  Serial.println("[HTTP] Actuador de interruptor: recibida orden de encender");
  server.send(200, "text/plain", "OK, actuador de interruptor encendido");
}

void manejadorApagarInterruptor() {
  estadoInterruptor = ESTADO_APAGADO;
  Serial.println("[HTTP] Actuador de enchufe: recibida orden de apagar");
  server.send(200, "text/plain", "OK, actuador de interruptor apagado");
}

void manejadorEstadoInterruptor() {
  if (estadoInterruptor) {
    Serial.println("[HTTP] Actuador de interruptor: recibida orden de petición de estado: encendido");
    server.send(200, "text/plain", "encender");
  } else {
    Serial.println("[HTTP] Actuador de interruptor: recibida orden de petición de estado: apagado");
    server.send(200, "text/plain", "apagar");
  }
}

void manejadorSubirPersiana() {
  estadoPersiana = 1;
  Serial.println("[HTTP] Actuador de persiana: recibida orden de subir");
  server.send(200, "text/plain", "OK, actuador de persiana subiendo");
}

void manejadorBajarPersiana() {
  estadoPersiana = 2;
  Serial.println("[HTTP] Actuador de persiana: recibida orden de bajar");
  server.send(200, "text/plain", "OK, actuador de persiana bajando");
}

void manejadorPararPersiana() {
  estadoPersiana = 0;
  Serial.println("[HTTP] Actuador de persiana: recibida orden de parar");
  server.send(200, "text/plain", "OK, actuador de persiana parado");
}

void paginaNoEncontrada() {
  String mensaje = "Página no encontrada\n\n";
  mensaje += "URI: ";
  mensaje += server.uri();
  mensaje += "\nMetodo: ";
  mensaje += (server.method() == HTTP_GET) ? "GET" : "POST";
  mensaje += "\nArgumentos: ";
  mensaje += server.args();
  mensaje += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    mensaje += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", mensaje);
}


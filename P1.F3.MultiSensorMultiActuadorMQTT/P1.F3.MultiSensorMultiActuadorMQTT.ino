/* Grupo 8
Alumnos: ESTEBAN y MIGUEL
*/

#include <WiFi.h>
#include <PubSubClient.h>  // Biblioteca para el cliente MQTT
#include "DHT.h"

//Constantes DHT11
#define DHTPIN 2
#define DHTTYPE DHT11

// Constantes MQTT
#define MSG_BUFFER_SIZE (50)
#define TIEMPO_ENTRE_MENSAJES (5000)
#define NUMERO_REINTENTOS (3)
#define TIEMPO_RECONEXION (5000)
#define OPCION_ABRIR "encender"
#define OPCION_BLOQUEAR "apagar"
#define ENCENDER HIGH
#define APAGAR LOW

//Topicos
#define TOPIC_SUBSCRIPCION_ENCHUFE "casa/dormitorio/enchufe/orden"
#define TOPIC_SUBSCRIPCION_INTERRUPTOR "casa/dormitorio/luztecho/orden"
#define TOPIC_PUBLICACION_INTERRUPTOR "casa/dormitorio/luztecho/estado"
#define TOPIC_PUBLICACION_DORMITORIO_TEMPERATURA "casa/dormitorio/temperatura"
#define TOPIC_PUBLICACION_SALON_TEMPERATURA "casa/salon/temperatura"
#define TOPIC_PUBLICACION_DORMITORIO_HUMEDAD "casa/dormitorio/humedad"
#define TOPIC_PUBLICACION_SALON_HUMEDAD "casa/salon/humedad"
#define TOPIC_PUBLICACION_DORMITORIO_SENSACION "casa/dormitorio/sensacionTermica"
#define TOPIC_PUBLICACION_SALON_SENSACION "casa/salon/sensacionTermica"


// Definición de constantes
const int ESTADO_APAGADO = LOW;
const int ESTADO_ENCENDIDO = HIGH;
const int PULSADOR_ESTADO_APAGADO = HIGH;   // PullUp
const int PULSADOR_ESTADO_ENCENDIDO = LOW;  // PullUp

// Pines de actuadores
const int PIN_ENCHUFE = 12;
const int PIN_INTERRUPTOR = 17;
const int PIN_PULSADOR_INTERRUPTOR = 22;

// Variables de estado
bool estadoEnchufe = ESTADO_APAGADO;
bool estadoInterruptor = ESTADO_APAGADO;
bool estadoPrevioPulsador = ESTADO_APAGADO;

// Otras variables
bool pulsadorEstabaPresionado = false;

// Parametros de conexión Wifi
const char *ssid = "LABDTE";
const char *password = "envev1d0s";

// IP de la ESP8266
IPAddress wifiIP(10, 49, 33, 81);
// Máscara de red
IPAddress wifiNET(255, 254, 0, 0);
// Dirección IP del encaminador
IPAddress wifiON(10, 48, 0, 1);
//IP del broker
IPAddress mqtt_server(10, 49, 33, 80);

const String clientId = "esp32Cliente-1";  // Identificador único de cliente. Cada dispositivo del hogar tiene que tener un identificador diferente

// Variables globales
WiFiClient clienteWIFI;
PubSubClient clienteMQTT(clienteWIFI);
char mensaje[MSG_BUFFER_SIZE];
long tiempoUltimoMensaje = 0;

//DHT11
DHT dht(DHTPIN, DHTTYPE);

//Inicializa los actuadores
void setupActuadores() {
  pinMode(PIN_ENCHUFE, OUTPUT);
  pinMode(PIN_INTERRUPTOR, OUTPUT);
  pinMode(PIN_PULSADOR_INTERRUPTOR, INPUT_PULLUP);
}

void setup() {
  setupActuadores();
  Serial.begin(115200);
  conectarWifi();
  clienteMQTT.setServer(mqtt_server, 1883);
  clienteMQTT.setCallback(manejador_mensajes);
  dht.begin();
}

void loop() {
  // Se asegura de que existe conexión con el broker
  if (!clienteMQTT.connected()) {
    conectar_MQTT();
  }

  // Se comprueban mensajes entrantes
  clienteMQTT.loop();
  loopActuadorEnchufe();
  loopActuadorInterruptor();
  loopMultisensor();
}

void conectarWifi() {
  delay(10);
  WiFi.mode(WIFI_STA);
  WiFi.config(wifiIP, wifiON, wifiNET);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conectado a WiFi!");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

/*
* Funcion encargada de la conexion con mqtt, intenta conectarse un numero definido de veces
* Si lo logra se suscribe a los topicos de interes, en este caso el enchufe e interruptor
* De no conectarse espera un tiempo determinado para volver a intentarlo
*/
void conectar_MQTT() {

  for (int i = 0; i < NUMERO_REINTENTOS; i++) {

    // Intento de conexión
    if (clienteMQTT.connect(clientId.c_str())) {
      Serial.println("Conectado");

      clienteMQTT.subscribe(TOPIC_SUBSCRIPCION_ENCHUFE);
      clienteMQTT.subscribe(TOPIC_SUBSCRIPCION_INTERRUPTOR);

      i = NUMERO_REINTENTOS;

    } else {

      Serial.print("Error al conectar. Resultado: ");
      Serial.print(clienteMQTT.state());
      Serial.println(". Se volverá a intentar en 5 segundos. ");

      delay(TIEMPO_RECONEXION);
    }
  }
}

/*
* Funcion encargada de manejar los mensajes, cuando llega un mensaje a uno de los topicos suscritos esta funcion es llamada
* Dado un mensaje se revisara su canal y se enviara a la funcion correspondiente
*/
void manejador_mensajes(char *canal, byte *mensaje, unsigned int longitud) {
  Serial.println("--------");
  Serial.print("Ha llegado un mensaje para el canal: ");
  Serial.println(canal);
  if (strcmp(canal, TOPIC_SUBSCRIPCION_ENCHUFE) == 0) {
    tratamiento_mensaje_enchufe(mensaje, longitud);
  } else if (strcmp(canal, TOPIC_SUBSCRIPCION_INTERRUPTOR) == 0) {
    tratamiento_mensaje_interruptor(mensaje, longitud);
  }
}


/**
 * Si el mensaje recibido corresponde al canal del enchufe esta funcion sera llamada
 * Segun el mensaje recibido se encendera o apagara el enchufe
*/
void tratamiento_mensaje_enchufe(byte *mensaje, unsigned int longitud) {

  char bufferTemporal[MSG_BUFFER_SIZE];
  strncpy(bufferTemporal, (char *)mensaje, longitud);
  bufferTemporal[longitud] = '\0';

  Serial.print("Mensaje recibido: ");
  Serial.println(bufferTemporal);

  if (strcmp(bufferTemporal, OPCION_ABRIR) == 0) {
    Serial.println("Se procede a encender la luz.");
    estadoEnchufe = ESTADO_ENCENDIDO;
    //digitalWrite(PIN_ENCHUFE, estadoEnchufe);
  } else if (strcmp(bufferTemporal, OPCION_BLOQUEAR) == 0) {
    Serial.println("Se procede a apagar la luz.");
    estadoEnchufe = ESTADO_APAGADO;
    
  } else {
    Serial.println("*** ERROR ***. Formato del mensaje recibido erróneo.");
  }
  Serial.println("--------");
}

/**
 * Si el mensaje recibido corresponde al canal del interruptor esta funcion sera llamada
 * Segun el mensaje recibido se encendera o apagara el interruptor
*/
void tratamiento_mensaje_interruptor(byte *mensaje, unsigned int longitud) {

  char bufferTemporal[MSG_BUFFER_SIZE];
  strncpy(bufferTemporal, (char *)mensaje, longitud);
  bufferTemporal[longitud] = '\0';

  Serial.print("Mensaje recibido: ");
  Serial.println(bufferTemporal);

  if (strcmp(bufferTemporal, OPCION_ABRIR) == 0) {
    Serial.println("Se procede a encender la luz.");
    estadoInterruptor = ESTADO_ENCENDIDO;
  } else if (strcmp(bufferTemporal, OPCION_BLOQUEAR) == 0) {
    Serial.println("Se procede a apagar la luz.");
    estadoInterruptor = ESTADO_APAGADO;
  } else {
    Serial.println("*** ERROR ***. Formato del mensaje recibido erróneo.");
  }

  Serial.println("--------");
}

/**
 * Funcion encargada de reflejar el estado del enchufe en el pin correspondiente
*/
void loopActuadorEnchufe() {
  digitalWrite(PIN_ENCHUFE, estadoEnchufe);
}

/**
 * Funcion encargada de monitorear el estado del pulsador del interruptor y cambiar la variable de estado segun corresponda
*/
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
  publicarEstadoPulsador();
}

/**
 * Funcion encargada de publicar en el topico correspondiente el estado del pulsador en intervalos designados
*/
void publicarEstadoPulsador() {
  long now = millis();
  if (now - tiempoUltimoMensaje > TIEMPO_ENTRE_MENSAJES) {
    // Se actualiza el último mensaje a la referencia de tiempo actual
    tiempoUltimoMensaje = now;

    if (estadoInterruptor == ESTADO_ENCENDIDO) {
    snprintf (mensaje, MSG_BUFFER_SIZE, "ON"); 
   } else {
    snprintf (mensaje, MSG_BUFFER_SIZE, "OFF");
   }
   Serial.print("Mensaje a publicar: "); 
   Serial.println(mensaje);

  mqtt_EnviarMensaje(TOPIC_PUBLICACION_INTERRUPTOR);
  }
}

void mqtt_EnviarMensaje(char* topic){
    clienteMQTT.publish(topic, mensaje); 
}

void loopMultisensor(){
  long now = millis();
  if (now - tiempoUltimoMensaje > TIEMPO_ENTRE_MENSAJES) {
    // Se actualiza el último mensaje a la referencia de tiempo actual
    tiempoUltimoMensaje = now;

    float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Celsius
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("temperatura a publicar: "); 
  sprintf(mensaje,"%f",t);
  Serial.println(mensaje);
  mqtt_EnviarMensaje(TOPIC_PUBLICACION_DORMITORIO_TEMPERATURA);
  Serial.print("humedad a publicar: "); 
  sprintf(mensaje,"%f",h);
  Serial.println(mensaje);
  mqtt_EnviarMensaje(TOPIC_PUBLICACION_DORMITORIO_HUMEDAD);
  Serial.print("sensacion termica a publicar: "); 
  sprintf(mensaje,"%f",hic);
  Serial.println(mensaje);
  mqtt_EnviarMensaje(TOPIC_PUBLICACION_DORMITORIO_SENSACION);
  }
}

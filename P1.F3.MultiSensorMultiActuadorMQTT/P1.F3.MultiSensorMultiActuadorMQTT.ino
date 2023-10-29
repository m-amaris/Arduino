/* Grupo 8
  Alumnos: ESTEBAN y MIGUEL

  Dispositivo compuesto por un multiactuador que se comunica por MQTT y un 
  multisensor que envía sus datos tanto por MQTT como por HTTP.

  El multiactuador está compuesto por un actuador de enchufe y un actuador de 
  interruptor. El multisensor está compuesto por un sensor de temperatura, uno 
  de humedad y otro de sensación térmica.
*/

#include <WiFi.h>          // Biblioteca para el módulo WiFi (ESP32)
#include <PubSubClient.h>  // Biblioteca para el cliente MQTT
#include <DHT.h>           // Biblioteca para el sensor DHT11

/* Definición de constantes y variables COMUNES al multiactuador y multisensor*/
#define ENCENDER HIGH
#define APAGAR LOW
const int ESTADO_APAGADO = LOW;
const int ESTADO_ENCENDIDO = HIGH;

// Parámetros de la red wifi
const char *ssid = "ASUS";
const char *password = "VIVAlosQUINTOSdel70!!!";

// Definición de las variables wifi y mqtt
IPAddress wifiIP(10, 49, 33, 81);         // IP de la ESP8266
IPAddress wifiNET(255, 254, 0, 0);        // Máscara de red
IPAddress wifiON(10, 48, 0, 1);           // Dirección IP del encaminador
IPAddress mqtt_server(192, 168, 1, 210);  //IP del broker
WiFiClient clienteWIFI;

#define MSG_BUFFER_SIZE (50)
#define TIEMPO_ENTRE_MENSAJES (5000)
#define NUMERO_REINTENTOS (3)
#define TIEMPO_RECONEXION (5000)

#define TOPIC_SUBSCRIPCION_ENCHUFE "casa/dormitorio/enchufe/orden"
#define TOPIC_SUBSCRIPCION_INTERRUPTOR "casa/dormitorio/luztecho/orden"
#define TOPIC_PUBLICACION_INTERRUPTOR "casa/dormitorio/luztecho/estado"
#define TOPIC_PUBLICACION_DORMITORIO_TEMPERATURA "casa/dormitorio/temperatura"
#define TOPIC_PUBLICACION_SALON_TEMPERATURA "casa/salon/temperatura"
#define TOPIC_PUBLICACION_DORMITORIO_HUMEDAD "casa/dormitorio/humedad"
#define TOPIC_PUBLICACION_SALON_HUMEDAD "casa/salon/humedad"
#define TOPIC_PUBLICACION_DORMITORIO_SENSACION "casa/dormitorio/sensacionTermica"
#define TOPIC_PUBLICACION_SALON_SENSACION "casa/salon/sensacionTermica"

const String clientId = "esp32Cliente-1";  // Identificador único de cliente. Cada dispositivo del hogar tiene que tener un identificador diferente
PubSubClient clienteMQTT(clienteWIFI);
char mensaje[MSG_BUFFER_SIZE];

/* Definición de constantes COMUNES del MULTIACTUADOR*/
#define OPCION_ENCENDER "encender"
#define OPCION_BLOQUEAR "apagar"
const int PULSADOR_ESTADO_APAGADO = HIGH;   // PullUp
const int PULSADOR_ESTADO_ENCENDIDO = LOW;  // PullUp
long tiempoUltimoMensajeMultiActuador = 0;

/* Definición de constantes y variables del ACTUADOR DE ENCHUFE */
#define PIN_ENCHUFE 12
bool estadoEnchufe = ESTADO_APAGADO;

/* Definición de constantes y variables del ACTUADOR DE INTERRUPTOR*/
#define PIN_INTERRUPTOR 17
#define PIN_PULSADOR_INTERRUPTOR 22
bool estadoInterruptor = ESTADO_APAGADO;
bool estadoPrevioPulsador = ESTADO_APAGADO;
bool pulsadorEstabaPresionado = false;

/* Definición de constantes y variables del MULTISENSOR */
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
long tiempoUltimoMensajeMultiSensor = 0;

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  setup_actuador_enchufe();
  setup_actuador_interruptor();
  setup_multisensor();

  conectarWifi();
  configurar_MQTT();

  Serial.println("Inicialización del dispositivo terminada.");
}

void setup_actuador_enchufe() {
  pinMode(PIN_ENCHUFE, OUTPUT);
}

void setup_actuador_interruptor() {
  pinMode(PIN_INTERRUPTOR, OUTPUT);
  pinMode(PIN_PULSADOR_INTERRUPTOR, INPUT_PULLUP);
}

void setup_multisensor() {
  dht.begin();
  Serial.println("Sensor DHT inicializado");
}

void conectarWifi() {
  delay(10);
  //WiFi.mode(WIFI_STA);                  // Comentar si se usa DHCP
  //WiFi.config(wifiIP, wifiON, wifiNET); // Comentar si se usa DHCP
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

void configurar_MQTT() {
  clienteMQTT.setServer(mqtt_server, 1883);
  clienteMQTT.setCallback(manejador_mensajes);
}

void loop() {
  clienteMQTT.loop();  // Se comprueban mensajes entrantes
  loopActuadorEnchufe();
  loopActuadorInterruptor();
  loop_multiActuadorMQTT();
  loopMultisensor();
}

/*
* Verifica si hay conexión con el broker. Si la hay, invoca a la función 
* conectar_MQTT() de los ejemplos y comprueba si llegan mensajes.
*/
void loop_multiActuadorMQTT() {
  // Se asegura de que existe conexión con el broker
  if (!clienteMQTT.connected()) {
    conectar_MQTT();
  }
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
      Serial.println("Conectado al broker");

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


/*
* Si el mensaje recibido corresponde al canal del enchufe esta funcion sera llamada
* Segun el mensaje recibido se encendera o apagara el enchufe
*/
void tratamiento_mensaje_enchufe(byte *mensaje, unsigned int longitud) {

  char bufferTemporal[MSG_BUFFER_SIZE];
  strncpy(bufferTemporal, (char *)mensaje, longitud);
  bufferTemporal[longitud] = '\0';

  Serial.print("Mensaje recibido: ");
  Serial.println(bufferTemporal);

  if (strcmp(bufferTemporal, OPCION_ENCENDER) == 0) {
    Serial.println("Se procede a encender el enchufe.");
    estadoEnchufe = ESTADO_ENCENDIDO;
    //digitalWrite(PIN_ENCHUFE, estadoEnchufe);
  } else if (strcmp(bufferTemporal, OPCION_BLOQUEAR) == 0) {
    Serial.println("Se procede a apagar el enchufe.");
    estadoEnchufe = ESTADO_APAGADO;

  } else {
    Serial.println("*** ERROR ***. Formato del mensaje recibido erróneo.");
  }
  Serial.println("--------");
}

/*
* Si el mensaje recibido corresponde al canal del interruptor esta funcion sera llamada
* Segun el mensaje recibido se encendera o apagara el interruptor
*/
void tratamiento_mensaje_interruptor(byte *mensaje, unsigned int longitud) {

  char bufferTemporal[MSG_BUFFER_SIZE];
  strncpy(bufferTemporal, (char *)mensaje, longitud);
  bufferTemporal[longitud] = '\0';

  Serial.print("Mensaje recibido: ");
  Serial.println(bufferTemporal);

  if (strcmp(bufferTemporal, OPCION_ENCENDER) == 0) {
    Serial.println("Se procede a encender el interruptor.");
    estadoInterruptor = ESTADO_ENCENDIDO;
  } else if (strcmp(bufferTemporal, OPCION_BLOQUEAR) == 0) {
    Serial.println("Se procede a apagar el interruptor.");
    estadoInterruptor = ESTADO_APAGADO;
  } else {
    Serial.println("*** ERROR ***. Formato del mensaje recibido erróneo.");
  }

  Serial.println("--------");
}

/*
* Funcion encargada de reflejar el estado del enchufe en el pin correspondiente
*/
void loopActuadorEnchufe() {
  digitalWrite(PIN_ENCHUFE, estadoEnchufe);
}

/*
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

/*
* Funcion encargada de publicar en el topic correspondiente el estado del pulsador en intervalos designados
*/
void publicarEstadoPulsador() {
  long now = millis();
  if (now - tiempoUltimoMensajeMultiActuador > TIEMPO_ENTRE_MENSAJES) {
    tiempoUltimoMensajeMultiActuador = now;  // Se actualiza el último mensaje a la referencia de tiempo actual
    if (estadoInterruptor == ESTADO_ENCENDIDO) {
      snprintf(mensaje, MSG_BUFFER_SIZE, "ON");
    } else {
      snprintf(mensaje, MSG_BUFFER_SIZE, "OFF");
    }
    Serial.print("Mensaje a publicar: ");
    Serial.println(mensaje);

    mqtt_EnviarMensaje(TOPIC_PUBLICACION_INTERRUPTOR);
  }
}

void mqtt_EnviarMensaje(char *topic) {
  clienteMQTT.publish(topic, mensaje);
}

/* Cada cierto tiempo toma medidas de los sensores y, si no hay errores, envía 
*  las medidas por mqtt y por http
*/
void loopMultisensor() {
  long now = millis();
  if (now - tiempoUltimoMensajeMultiSensor > TIEMPO_ENTRE_MENSAJES) {
    tiempoUltimoMensajeMultiSensor = now;  // Se actualiza el último mensaje a la referencia de tiempo actual

    Serial.print("Tomando medidas del sensor ");
    //float h = dht.readHumidity();
    //float t = dht.readTemperature();  // Read temperature as Celsius (the default)
    float h = random(0, 1000) / 10;       // a random decimal number from 0.00 to 0.99
    float t = random(-1000, 5100) / 100;   // a random integer from -10 to 50

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println();
      Serial.println(F("Error al leer del sensor de temperatura"));
      Serial.println(F("Error al leer del sensor de humedad"));
      Serial.println(F("Error al leer del sensor!"));
      return;
    }
    float hic = dht.computeHeatIndex(t, h, false);  // Compute heat index in Celsius

    Serial.print("Temperatura: ");
    Serial.print(t,2);
    Serial.print("°C ");

    Serial.print("Humedad: ");
    Serial.print(h,2);
    Serial.print("°C ");

    Serial.print("Sensación Térmica: ");
    Serial.print(hic,2);
    Serial.print("°C");

    Serial.println();
    Serial.print("[MQTT] Enviar el dato ");
    sprintf(mensaje, "%.2f", t);
    Serial.print(mensaje);
    Serial.print(" al topic ");
    Serial.println(TOPIC_PUBLICACION_DORMITORIO_TEMPERATURA);
    mqtt_EnviarMensaje(TOPIC_PUBLICACION_DORMITORIO_TEMPERATURA);

    Serial.print("[MQTT] Enviar el dato ");
    sprintf(mensaje, "%.2f", h);
    Serial.print(mensaje);
    Serial.print(" al topic ");
    Serial.println(TOPIC_PUBLICACION_DORMITORIO_HUMEDAD);
    mqtt_EnviarMensaje(TOPIC_PUBLICACION_DORMITORIO_HUMEDAD);

    Serial.print("[MQTT] Enviar el dato ");
    sprintf(mensaje, "%.2f", hic);
    Serial.print(mensaje);
    Serial.print(" al topic ");
    Serial.println(TOPIC_PUBLICACION_DORMITORIO_SENSACION);
    mqtt_EnviarMensaje(TOPIC_PUBLICACION_DORMITORIO_SENSACION);

    Serial.println("---------------------------");
  }
}

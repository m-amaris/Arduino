/* Grupo 8
  Alumnos: ESTEBAN y MIGUEL

  Dispositivo compuesto por un multiactuador que se comunica por MQTT y un
  multisensor que envía sus datos tanto por MQTT como por HTTP.
  
  El multiactuador está compuesto por un actuador de enchufe y un actuador de
  interruptor. El multisensor está compuesto por un sensor de temperatura, uno
  de humedad y otro de sensación térmica.
*/

#include <WiFi.h>
#include <PubSubClient.h>  // Biblioteca para el cliente MQTT
#include <DHT.h>           // Biblioteca para el sensor DHT11

/* Definición de constantes y variables COMUNES al multiactuador y multisensor*/
#define ENCENDER HIGH
#define APAGAR LOW
const int ESTADO_APAGADO = LOW;
const int ESTADO_ENCENDIDO = HIGH;

// Parámetros de la red wifi
const char *ssid = "ADSL50";
const char *password = "Mercad02";

// Definición de las variables wifi y mqtt
IPAddress wifiIP(10, 49, 33, 81);         // IP de la ESP8266
IPAddress wifiNET(255, 255, 255, 0);        // Máscara de red
IPAddress wifiON(10, 48, 0, 1);           // Dirección IP del encaminador
IPAddress mqtt_server(192,168,1,38);  //IP del broker
WiFiClient clienteWIFI;

#define MSG_BUFFER_SIZE (50)
#define TIEMPO_ENTRE_MENSAJES (5000)
#define NUMERO_REINTENTOS (3)
#define TIEMPO_RECONEXION (5000)

#define TOPIC_SUBSCRIPCION_ENCHUFE "casa/dormitorio/enchufe/orden"
#define TOPIC_SUBSCRIPCION_INTERRUPTOR "casa/dormitorio/luztecho/orden"
#define TOPIC_PUBLICACION_INTERRUPTOR "casa/dormitorio/luztecho/estado"
#define TOPIC_PUBLICACION_DORMITORIO_TEMPERATURA "casa/dormitorio/temperatura"
#define TOPIC_PUBLICACION_DORMITORIO_HUMEDAD "casa/dormitorio/humedad"
#define TOPIC_PUBLICACION_DORMITORIO_SENSACION "casa/dormitorio/sensacionTermica"

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
#define PIN_ENCHUFE 15
bool estadoEnchufe = ESTADO_APAGADO;

/* Definición de constantes y variables del ACTUADOR DE INTERRUPTOR*/
#define PIN_INTERRUPTOR 18
#define PIN_PULSADOR_INTERRUPTOR 22
bool estadoInterruptor = ESTADO_APAGADO;
bool estadoPrevioPulsador = ESTADO_APAGADO;
bool pulsadorEstabaPresionado = false;

/* Definición de constantes y variables del MULTISENSOR */
#define RUTA_sensor_temperatura_salon "/rest/items/Salon_Temperatura"
#define RUTA_sensor_humedad_salon "/rest/items/Salon_Humedad" 
#define RUTA_sensor_sensacion_salon "/rest/items/Salon_SensacionTermica"
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
long tiempoUltimoMensajeMultiSensor = 0;


void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  setup_multisensor();

  conectar_WIFI();
  configurar_MQTT();

  Serial.println("Inicialización del dispositivo terminada.");
}


void setup_multisensor() {
  dht.begin();
  Serial.println("Sensor DHT inicializado");
}

void conectar_WIFI() {
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

/* 
* Verifica si el microcontrolador sigue conectado a la wifi
* Devuelve un valor boolean: true si está conectado y false si no lo está
*/
bool estoy_conectado() {
  return (WiFi.status() == WL_CONNECTED);
}

void configurar_MQTT() {
  clienteMQTT.setServer(mqtt_server, 1883);
  clienteMQTT.setCallback(manejador_mensajes);
}

void loop() {
  clienteMQTT.loop(); // Se comprueban mensajes entrantes
  loop_multiActuadorMQTT();
  loop_multiSensor();
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
}


void mqtt_EnviarMensaje(char *topic) {
  clienteMQTT.publish(topic, mensaje);
}

/* 
*  Cada cierto tiempo toma medidas de los sensores y, si no hay errores, envía 
*  las medidas por mqtt y por http
*/
void loop_multiSensor() {
  long now = millis();
  if (now - tiempoUltimoMensajeMultiSensor > TIEMPO_ENTRE_MENSAJES) {
    tiempoUltimoMensajeMultiSensor = now;  // Se actualiza el último mensaje a la referencia de tiempo actual

    Serial.println();
    Serial.print("Tomando medidas del sensor ");
    float h = dht.readHumidity();
    float t = dht.readTemperature();  // Read temperature as Celsius (the default)
    //float h = random(0, 1000) / 10;       // a random decimal number from 0.00 to 0.99
    //float t = random(-1000, 5100) / 100;   // a random integer from -10 to 50

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
  }
}

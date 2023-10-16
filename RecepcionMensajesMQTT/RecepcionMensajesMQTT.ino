
/***  

  Programa para recepción de mensajes mediante MQTT a través de un ESP8266

***/

#include <WiFi.h> 
#include <PubSubClient.h> // Biblioteca para el cliente MQTT

// Constantes programa
#define MSG_BUFFER_SIZE  (50)
#define NUMERO_REINTENTOS (3)
#define TIEMPO_RECONEXION (5000)
#define OPCION_ABRIR "encender"
#define OPCION_BLOQUEAR "apagar"
#define TOPIC_SUBSCRIPCION "casa/dormitorio/luztecho/orden" 
#define ENCENDER HIGH
#define APAGAR LOW

// Pines de actuadores
const int PIN_ENCHUFE = 17;

// Parametros de conexión Wifi
const char* ssid = "iPhone de Miguel";
const char* password = "envev1d0s";

// IP de la ESP8266
IPAddress wifiIP(172, 20, 10, 4); 
// Máscara de red
IPAddress wifiNET(255, 255, 255, 240);
// Dirección IP del encaminador
IPAddress wifiON(172, 20, 10, 1); 
//IP del broker 
IPAddress mqtt_server (172, 20, 10, 3); 


const String clientId = "esp32Cliente-1";   // Identificador único de cliente. Cada dispositivo del hogar tiene que tener un identificador diferente

// Variables globales
WiFiClient clienteWIFI; 
PubSubClient clienteMQTT(clienteWIFI);

void setup() {
  // Puerto serie
  Serial.begin(115200);
  // Se habilita la conexión Wifi
  conectar_WIFI(); 

    // Se establece la dirección del Broker para MQTT
  clienteMQTT.setServer(mqtt_server, 1883); 
  
  // Se establece la función de callback
  clienteMQTT.setCallback(manejador_mensajes);
  pinMode(PIN_ENCHUFE, OUTPUT);
}

void loop() { 

  // Se asegura de que existe conexión con el broker
  if (!clienteMQTT.connected()) {
    conectar_MQTT();
  }

  // Se comprueban mensajes entrantes
  clienteMQTT.loop();
 
}

void conectar_WIFI() {
  
  // Da tiempo a que se inicialice el hardware de la Wifi
  delay(10);
  
  // Establecer configuración wifi para no usar DHCP
  WiFi.mode(WIFI_STA);
  
  //Para no usar DHCP hay que definir la red. 
  WiFi.config(wifiIP,wifiON,wifiNET); 
  WiFi.begin(ssid, password);  
  // Serial.printf("Conectándo a la wifi con SSID %s y clave %s",ssid,password );   
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Conectado a WiFi!");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
}


void conectar_MQTT() {
  
  for (int i=0 ; i<NUMERO_REINTENTOS ; i++) {

    // Intento de conexión
    if (clienteMQTT.connect(clientId.c_str())) {
      Serial.println("Conectado");
      
      clienteMQTT.subscribe(TOPIC_SUBSCRIPCION);
      
      i=NUMERO_REINTENTOS;
    
    } else {
      
      Serial.print("Error al conectar. Resultado: ");
      Serial.print(clienteMQTT.state());
      Serial.println(". Se volverá a intentar en 5 segundos. ");
      
      delay(TIEMPO_RECONEXION);
    }
  }
}

void manejador_mensajes (char *canal, byte *mensaje, unsigned int longitud) {
  Serial.print("Ha llegado un mensaje para el canal: ");
  Serial.println(canal);
  tratamiento_mensaje (mensaje, longitud);
}

void tratamiento_mensaje (byte *mensaje, unsigned int longitud) {

  char bufferTemporal [MSG_BUFFER_SIZE];
  strncpy (bufferTemporal, (char *) mensaje, longitud);
  bufferTemporal [longitud] = '\0';
  
  Serial.print("Mensaje recibido: ");
  Serial.println(bufferTemporal);

  if (strcmp(bufferTemporal,OPCION_ABRIR) == 0) {
    Serial.println("Se procede a encender la luz.");
    digitalWrite(PIN_ENCHUFE, ENCENDER);
  } else if (strcmp(bufferTemporal,OPCION_BLOQUEAR) == 0) {
    Serial.println("Se procede a apagar la luz.");
    digitalWrite(PIN_ENCHUFE, APAGAR);
  } else {
    Serial.println("*** ERROR ***. Formato del mensaje recibido erróneo.");
  }
    
  Serial.println("--------");
}

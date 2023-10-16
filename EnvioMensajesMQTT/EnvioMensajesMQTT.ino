
/***  

  Programa para envío de mensajes mediante MQTT a través de un ESP8266

***/

#include <WiFi.h> 
#include <PubSubClient.h> // Biblioteca para el cliente MQTT

// Constantes programa
#define MSG_BUFFER_SIZE  (50)
#define TIEMPO_ENTRE_MENSAJES (2000)
#define NUMERO_REINTENTOS (5)
#define TIEMPO_RECONEXION (5000)
#define PRESENCIA "ON"
#define NO_PRESENCIA "OFF"
#define TOPIC_PUBLICACION "casa/dormitorio/luztecho/estado" 

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


const String clientId = "esp32Cliente-1"; 	// Identificador único de cliente. Cada dispositivo del hogar tiene que tener un identificador diferente

// Variables globales
WiFiClient clienteWIFI; 
PubSubClient clienteMQTT(clienteWIFI);
char mensaje[MSG_BUFFER_SIZE];
long tiempoUltimoMensaje = 0;


void setup() {
  // Puerto serie
  Serial.begin(115200);
 
  // Se habilita la conexión Wifi
  conectar_WIFI(); 

  // Se establece la dirección del Broker para MQTT
  clienteMQTT.setServer(mqtt_server, 1883); 

  // Se inicializa el generador de pseudoaleatoriedad
  randomSeed(micros());

}

void loop() { 

  // Se asegura de que existe conexión con el broker
  if (!clienteMQTT.connected()) {
    conectar_MQTT();
  }

  // Control de envíos espaciados en el tiempo (TIEMPO_ENTRE_MENSAJES)
  // Se obtiene una referencia de tiempo actual
  long now = millis();  
  if (now - tiempoUltimoMensaje > TIEMPO_ENTRE_MENSAJES) {   
    // Se actualiza el último mensaje a la referencia de tiempo actual
    tiempoUltimoMensaje = now;    
    detecta_presencia (); 
    mqtt_EnviarMensaje(); 
   }
 
}

void conectar_WIFI() {
  
  // Da tiempo a que se inicialice el hardware de la Wifi
  delay(10);
  
  // Establecer configuración wifi para no usar DHCP
  WiFi.mode(WIFI_STA);
  
  //Para no usar DHCP hay que definir la red. 
  WiFi.config(wifiIP,wifiON,wifiNET); 
  WiFi.begin(ssid, password);  
  Serial.printf("Conectándo a la wifi con SSID %s y clave %s",ssid,password );   
  
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
      i=NUMERO_REINTENTOS;
    
    } else {
      Serial.print("Error al conectar. Resultado: ");
      Serial.print(clienteMQTT.state());
      Serial.println(". Se volverá a intentar en 5 segundos. ");
      
      delay(TIEMPO_RECONEXION);
    }
  }
}

void detecta_presencia () {
   long numeroAleatorio = random(0, 2);
   Serial.print("Numero seleccionado: "); 
   Serial.println (numeroAleatorio); 
   if (numeroAleatorio) {
    snprintf (mensaje, MSG_BUFFER_SIZE, PRESENCIA); 
   } else {
    snprintf (mensaje, MSG_BUFFER_SIZE, NO_PRESENCIA);
   }
   Serial.print("Mensaje a publicar: "); 
   Serial.println(mensaje);
}

void mqtt_EnviarMensaje(){
    clienteMQTT.publish(TOPIC_PUBLICACION, mensaje); 
}

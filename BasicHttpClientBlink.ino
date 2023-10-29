/**
   BasicHTTPClientBlink.ino
   Author: UPM
   Date: 2023
   Este sketch parte del ConexiónWiFiSinDHCP para realizar la conexión WiFi y del 
   BasicHTTPClientGetYPost del que toma el código para hacer las peticiones GET y POST.
   Se comporta como el ejemplo del blink, encendido y apagado el led interno de la placa según correspond
   y además añadiendo la funcionalidad del envío periódico de los mensajes de encendido/apagado al servidor web.
   Además se ha realizado empleando funciones y constantes
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

// Definición de constantes
// Se configura como constantes los datos de identificación de la red WiFi
const char* ssid     = "xxxxxxxxxxxxxx"; // Establecer SSID red WiFi de tu casa
const char* password = "xxxxxxxxxxxxxx"; // Establecer contraseña red WiFi de tu casa


// Se define en una constante la IP asignada a la máquina virtual donde corre el Servidor Web
const String IPServidorWeb = "192.168.0.91";

// Se definen dos constantes para entender mejor lo que significa Encender o Apagar
#define APAGAR HIGH
#define ENCENDER LOW
// Se definen dos constantes para saber el estado del led interno
#define ENCENDIDO true
#define APAGADO false

// Definición de variables globales
// Asignación de una IP fija en vez de usar el protocolo DHCP
IPAddress wifiIP(192, 168, 0, 90);    // IP que se le asigna al microcontrolador
IPAddress wifiNET(255, 255, 255, 0);  // Máscara de red de la red WiFi
IPAddress wifiON(192, 168, 0, 1);     // IP del Router de la red WiFi
HTTPClient clienteHTTP;               // Variable que se usará como cliente HTTP
bool estadoLedInterno;                // Variable para controlar el estado de encendido o apagado del led interno

void setup() {
  // Se inicializa el pin digital LED_BUILTIN como una salida
  pinMode(LED_BUILTIN, OUTPUT);

  // Se configura la velocidad a la que se transmite la información desde la placa hasta la consola.
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  conectar_WIFI();

  // Encender y apagar el led interno varias veces para así indicar que se está arrancando el programa
  for (uint8_t t = 4; t > 0; t--) {
    digitalWrite(LED_BUILTIN, ENCENDER);
    delay(100);
    digitalWrite(LED_BUILTIN, APAGAR);
    delay(100);
  }

  estadoLedInterno = APAGADO;  // Al terminar el setup el estado del led interno es apagado
}

void loop() {
  // Se conmuta el estado del led interno, si está encendido se apaga y si está apagado se enciende
  estadoLedInterno = !estadoLedInterno;

  if (estadoLedInterno == APAGADO) {    // Si debe estar apagado el led interno
    digitalWrite(LED_BUILTIN, APAGAR);  // Se apaga el led interno
                                        // Se envía el mensaje "apagado" al Servidor Web, empleando POST, indicando que se ha apagado el led interno
    envioDatoHTTPPost("apagado");
  } else {                                // Si debe estar encendido el led interno
    digitalWrite(LED_BUILTIN, ENCENDER);  // Se enciende el led interno
    // Se envía un "encendido" empleando GET al Servidor Web indicando que se ha encendido el led interno
    envioDatoHTTPGet("encendido");
  }
  delay(2000);  // Se muestra el mensaje cada 2 segundos
}

/* Función conectar_WIFI () introducida en el Módulo 2 del curso
   Se conecta a la wifi hasta conseguirlo, sin usar el protocolo DHCP
   Cuando termina, muestra la IP obtenida
*/
void conectar_WIFI() {
  Serial.print("Conectando con ");
  Serial.print(ssid);
  Serial.print(" ");
  // Da tiempo a que se inicialice el hardware de la Wifi
  delay(10);

  // Establecer configuración wifi para no usar DHCP
  WiFi.mode(WIFI_STA);


  // Se establecen los parámetros de red sin usar DHCP
  WiFi.config(wifiIP, wifiON, wifiNET);
  WiFi.begin(ssid, password);
  // Intentar la conexión cada medio segundo hasta conseguirlo, mostrando un punto en cada intento
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println(" WiFi conectada");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

/* Función estoyConectado()
   Verifica si el microcontrolador sigue conectado a la wifi
   Devuelve un valor boolean: true si está conectado y false si no lo está
*/
bool estoyConectado() {
  return (WiFi.status() == WL_CONNECTED);
}

/* Función envioDatoHTTPGet()
   Se emplea para enviar por GET al Servidor Web el dato que se recibe como parámetro
*/
void envioDatoHTTPGet(String dato) {
  WiFiClient clienteWiFi;
  if (!estoyConectado())
    // Si se ha perdido la conexión con la red WiFi se vuelve a establecer
    conectar_WIFI();

  Serial.print("[HTTP] comienzo...\n");
  if (clienteHTTP.begin(clienteWiFi, "http://" + IPServidorWeb + ":8080/probando?dato=" + dato)) {
    // Conexión HTTP iniciada
    Serial.print("[HTTP] GET...\n");
    int httpCode = clienteHTTP.GET();  // Recuperar el Código de respuesta del servidor
    // httpCode debe ser un valor positivo, en caso contrario es un error
    if (httpCode > 0) {
      // Mostrar el código devuelto
      Serial.printf("[HTTP] GET... código: %d\n", httpCode);

      // Si el servidor devuelve un código de que la petición es correcta
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = clienteHTTP.getString();
        Serial.println(payload);  // Se muestra por consola la respuesta del servidor
      }
    } else {
      Serial.printf("[HTTP] GET... fallo, código de error: %s\n", clienteHTTP.errorToString(httpCode).c_str());
    }
    clienteHTTP.end();
  } else {
    Serial.printf("[HTTP} Falló la conexión HTTP\n");
  }
}

/* Función envioDatoHTTPPostt()
   Se emplea para enviar por POST al Servidor Web el dato que se recibe como parámetro
*/
void envioDatoHTTPPost(String dato) {
  WiFiClient clienteWiFi;
  if (!estoyConectado())
    // Si se ha perdido la conexión con la red WiFi se vuelve a establecer
    conectar_WIFI();

  Serial.print("[HTTP] comienzo...\n");
  if (clienteHTTP.begin(clienteWiFi, "http://" + IPServidorWeb + ":8080/probando")) {
    // Conexión HTTP iniciada
    Serial.print("[HTTP] POST...\n");
    clienteHTTP.addHeader("Content-Type", "text/plain");  // Añadir cabecera a la petición POST
    int httpCode = clienteHTTP.POST(dato);
    // httpCode debe ser un valor positivo, en caso contrario es un error
    if (httpCode > 0) {
      // Mostrar el código devuelto
      Serial.printf("[HTTP] POST... código: %d\n", httpCode);

      // Si el servidor devuelve un código de que la petición es correcta
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = clienteHTTP.getString();
        Serial.println(payload);  // Se muestra por consola la respuesta del servidor
      }
    } else {
      Serial.printf("[HTTP] POST... fallo, código de error: %s\n", clienteHTTP.errorToString(httpCode).c_str());
    }
    clienteHTTP.end();
  } else {
    Serial.printf("[HTTP} Falló la conexión HTTP\n");
  }
}

/*
 *    ConexiónWiFiSinDHCP.ino
      Author: UPM
      Date: 2021
      Conexión a la WiFi de la casa SIN EMPLEAR DHCP
*/

#include <ESP8266WiFi.h>

// Definición de constantes
// Paso 1
// Se configura como constantes los datos de identificación de la red WiFi
const char* ssid     = "ASUS"; // Establecer SSID red WiFi de tu casa 
const char* password = "VIVAlosQUINTOSdel70!!!"; // Establecer contraseña red WiFi de tu casa

// Definición de variables globales
// NUEVO
// Inicializar las variables con la información de red puesto que no se va a usar el protocolo DHCP
IPAddress wifiIP(192, 168, 1, 232);  // IP que se asignará al microcontrolador (debe ser una IP sin uso en la red)
IPAddress wifiNET (255, 255, 255, 0); // Máscara de Red
IPAddress wifiON (192, 168, 1, 1); // Dirección IP del router 

//Paso 2
void setup() {
  // Se configura la velocidad a la que se transmite la información desde la placa hasta la consola.
  Serial.begin(115200);
  // Se implementa la configuración de la WiFi en una función
  // Esto limita el tamaño de la función setup
  conexionWifi();
}

void loop() {
  // Paso 5
  if (!estoyConectado()){
     // Si se ha perdido la conexión se vuelve a realizar la conexión a la red Wifi
     conexionWifi();
  }
  else{
     Serial.println("Seguimos correctamente conectados a la WiFi");
  }
  delay(2000); // Se muestra el mensaje cada 2 segundos
}

//Paso 3
/* Función conexionWifi()
   Se conecta a la wifi hasta conseguirlo, sin usar el protocolo DHCP
   Cuando termina, muestra la IP obtenida
*/
void conexionWifi(){
  Serial.println();
  Serial.println();
  Serial.print("Conectando con ");
  Serial.println(ssid);
  
  // Conexión a la red WiFi
  // Se establece el microcontrolador como un cliente WiFi
  WiFi.mode(WIFI_STA);

  // NUEVO
  // Se establecen los parámetros de red sin usar DHCP
  WiFi.config(wifiIP, wifiON, wifiNET); 
  
  WiFi.begin(ssid, password);
  // Intentar la conexión cada medio segundo hasta conseguirlo, mostrando un punto en cada intento
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectada");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
}


// Paso 4
/* Función estoyConectado()
   Verifica si el microcontrolador sigue conectado a la wifi
   Devuelve un valor boolean: true si está conectado y false si no lo está
*/
bool estoyConectado() {
    return (WiFi.status() == WL_CONNECTED);
}

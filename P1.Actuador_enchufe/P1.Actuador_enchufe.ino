#include <ESP8266WiFi.h>        // biblioteca wifi de esp8266
#include <ESP8266WebServer.h>   // Servidor web

// Definición de constantes
// Led interno - Solo para pruebas
#define LEDINTERNO LED_BUILTIN
#define ENCENDER LOW
#define APAGAR HIGH
// Relé
#define APAGAR_ENCHUFE LOW
#define ENCENDER_ENCHUFE HIGH
#define PINRELE_ENCHUFE 13  // Pin donde irá conectado el relé

// Parámetros de la red wifi de la casa
const char* ssid     = "ASUS"; // Establecer SSID red WiFi de tu casa 
const char* password = "VIVAlosQUINTOSdel70!!!"; // Establecer contraseña red WiFi de tu casa

// IP de la ESP8266
IPAddress wifiIP(192, 168, 1, 232); 
// Máscara de red
IPAddress wifiNET(255, 255, 255, 0);
// Dirección IP del encaminador
IPAddress wifiON(192, 168, 1, 1); 

// Objecto servidor
ESP8266WebServer servidorWeb(80);       
bool enchufeEncendido;

void setup(void) {
  // Configuración de pines
  pinMode(LEDINTERNO, OUTPUT);               // Configurar pin como salida
  digitalWrite(LEDINTERNO, APAGAR);          // Apagar led interno
  
  pinMode(PINRELE_ENCHUFE, OUTPUT);           // Configurar pin como salida
  digitalWrite(PINRELE_ENCHUFE, APAGAR_ENCHUFE);  // Desactivar relé
  enchufeEncendido = false;
  
  Serial.begin(115200);           // Establecer velocidad consola serie

  conectar_WIFI ();

  // Configuración del servidor web:
  configura_ServidorWEB ();

  // Arrancar servidor web
  servidorWeb.begin();
  
  Serial.println("Servidor web arrancado");
  Serial.println("Listo. Conectarse a un navegador y usar estas URLs:");
  Serial.print("Para activar: ");
  Serial.print (WiFi.localIP());
  Serial.println("/activar");
  Serial.print("Para desactivar: ");
  Serial.print (WiFi.localIP());
  Serial.println("/desactivar");

}

void loop(void) {
  // Consultar si se ha recibido una petición al servidor web
  servidorWeb.handleClient();     

  // POSIBLE MODIFICACIÓN
  comprobar_pulsacion ();
  
}

void comprobar_pulsacion () {
  // Lectura del pin de control del pulsados
  // Filtrar por software los rebotes del pulsador
  // Si se ha activado el pulsador actualizar los pines y variables de estado
}

void configura_ServidorWEB () {

  /* 
   * POSIBLE MOFICACIÓN: Con distintas URLs tendríamos posibilidad de controlar 
   *  distintos pines (circuitos) de forma sencilla
   */
  
  servidorWeb.on("/", manejadorRaiz);                   
  servidorWeb.on("/enchufe/encender", manejadorEncenderEnchufe);
  servidorWeb.on("/enchufe/apagar", manejadorApagarEnchufe);
  servidorWeb.on("/enchufe/estado", manejadorEstado);
  servidorWeb.onNotFound(paginaNoEncontrada);
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

// Funciones asociadas a cada petición
void manejadorEstado() {

  if (enchufeEncendido) {
    servidorWeb.send(200, "text/plain", "encendido");  
  } else {
    servidorWeb.send(200, "text/plain", "apagado");
  }
  
}

void manejadorApagarEnchufe() {
  digitalWrite(LEDINTERNO, APAGAR);          // Apagar led interno
  digitalWrite(PINRELE_ENCHUFE, APAGAR_ENCHUFE);  // Desactivar relé
  enchufeEncendido = false;
  servidorWeb.send(200, "text/plain", "OK, actuador de enchufe apagado");
  Serial.println("Relé desactivado");
}

// Funciones asociadas a cada petición
void manejadorEncenderEnchufe() {
  digitalWrite(LEDINTERNO, ENCENDER);      // Encender led interno
  digitalWrite(PINRELE_ENCHUFE, ENCENDER_ENCHUFE);   // Activar relé
  enchufeEncendido = true;
  servidorWeb.send(200, "text/plain", "OK, actuador de enchufe encendido");
  Serial.println("Relé activado");
}

// Página inicial con el menú de opciones
void manejadorRaiz() {
  String mensaje;
  mensaje = "<!DOCTYPE HTML>\r\n<html>\r\n";
  mensaje += "<head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";
  mensaje += "<title>Actuador de enchufe</title></head>\r\n";
  mensaje += "<body>";
  mensaje += "Pulsa para <a href=""/enchufe/encender"">Encender</a><br>";
  mensaje += "Pulsa para <a href=""/enchufe/apagar"">Apagar</a><br>";
  mensaje += "Pulsa para <a href=""/enchufe/estado"">Consulta de Estado</a>";
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

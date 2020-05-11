#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <ESP8266mDNS.h>

#define APSTASSID "ESPap"
#define APSTAPSK  "khaled112358"


const char *ap_ssid = APSTASSID;
const char *ap_password = APSTAPSK;

//Commands
const char toggleSTMLED_Command[] = "$toggleSTMLED";
const char toggleESPLED_Command[] = "$toggleESPLED";
const char LEDOFF[] = "ledoff";

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

const int ESPLed = LED_BUILTIN;

bool LEDSTMState = false;
bool LEDESPState = false;
void toggleSTMLED();
void toggleESPLED();
const bool DEBUG = false;
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);

bool handleFileRead(String path);

String getContentType(String filename);

void setup(void) {
  pinMode(ESPLed, OUTPUT);
  digitalWrite(ESPLed, 0);

  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  if (DEBUG) {
    Serial.println();
    Serial.println("Configuring access point...");
  }
  WiFi.softAP(ap_ssid, ap_password);
  IPAddress myIP = WiFi.softAPIP();
  if (DEBUG) {
    Serial.print("AP IP address: ");
    Serial.println(myIP);
  }

  if (MDNS.begin("esp8266") && DEBUG) {
    Serial.println("MDNS responder started");
  }

  SPIFFS.begin();

  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "404: Not Found");
  });
  server.begin();

  if (DEBUG) {
    Serial.println("HTTP server started");
  }

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop(void) {
  webSocket.loop();
  server.handleClient();
  MDNS.update();
}

void transmit_delayed(const char* trx_buffer, int period) {
  for (int i = 0; i < strlen(trx_buffer); i++) {
    Serial.print(trx_buffer[i]);
    delay(period);
    Serial.flush();
  }
}

void toggleSTMLED() {
  LEDSTMState = !LEDSTMState;
  if (DEBUG) {
    Serial.printf("STM LED: %u!\r\n", LEDSTMState);
  }
  transmit_delayed(toggleSTMLED_Command,10);
}

void toggleESPLED() {
  LEDESPState = !LEDESPState;
  if (DEBUG) {
    Serial.printf("ESP LED: %u\r\n", LEDESPState);
  }
  digitalWrite(ESPLed, LEDESPState ? HIGH : LOW);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (DEBUG) {
    Serial.printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
  }
  switch (type) {
    case WStype_DISCONNECTED:
      if (DEBUG) {
        Serial.printf("[%u] Disconnected!\r\n", num);
      }
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        if (DEBUG) {
          Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        }
        // Send the current LED status
      }
      break;
    case WStype_TEXT:
      if (DEBUG) {
        Serial.printf("[%u] get Text: %s\r\n", num, payload);
      }

      if (strcmp(toggleESPLED_Command, (const char *)payload) == 0) {
        toggleESPLED();
      } else if (strcmp(toggleSTMLED_Command, (const char *)payload) == 0) {
        toggleSTMLED();
      } else if (DEBUG) {
        Serial.println("Unknown command");
      }

      webSocket.broadcastTXT(payload, length);
      if (DEBUG) {
        printf("[%u]   payload- %s\n", num, payload);
      }
      break;
    case WStype_BIN:
      if (DEBUG) {
        Serial.printf("[%u] get binary length: %u\r\n", num, length);
        hexdump(payload, length);
      }
      // echo data back to browser
      webSocket.sendBIN(num, payload, length);
      break;
    default:
      if (DEBUG) {
        Serial.printf("Invalid WStype [%d]\r\n", type);
      }
      break;
  }
}

//void handleRoot() {
//  digitalWrite(led, 1);
////  int sec = millis() / 1000;
////  int min = sec / 60;
////  int hr = min / 60;
//  int htmlSize = strlen(INDEX_HTML);
//  server.setContentLength(htmlSize);
//  server.send(200, "text/html", "");
//  int bufferCounter = 0;
//  int bufferSize = 6000;
//  int chunks = htmlSize/bufferSize;
//  int remainder = htmlSize%bufferSize;
//  Serial.printf("bufferCounter = [%u], bufferSize = [%u], chunks = [%u], remainder = [%u]\r\n", bufferCounter, bufferSize, chunks, remainder);
//  for(int i=0;i<chunks;i++){
//    Serial.printf("bufferCounter = [%u]\r\n",bufferCounter);
//    server.sendContent_P(&INDEX_HTML[bufferCounter], bufferSize); //send the current buffer
//    bufferCounter+=bufferSize;
//  }
//  Serial.printf("bufferCounter = [%u]\r\n",bufferCounter);
//  server.sendContent_P(&INDEX_HTML[bufferCounter], remainder); //send the current buffer
//  digitalWrite(led, 0);
//}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  if (DEBUG) {
    Serial.println("handleFileRead: " + path);
  }
  if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                         // If there's a compressed version available
      path += ".gz";                                         // Use the compressed version
    File file = SPIFFS.open(path, "r");                    // Open the file
    size_t sent = server.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    if (DEBUG) {
      Serial.println(String("\tSent file: ") + path);
    }
    return true;
  }
  // Close the file again
  if (DEBUG) {
    Serial.println(String("\tFile Not Found: ") + path);
  }
  return false;                                          // If the file doesn't exist, return false
}

String getContentType(String filename) {
  if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

//
//void handleNotFound() {
//  digitalWrite(led, 1);
//  String message = "File Not Found\n\n";
//  message += "URI: ";
//  message += server.uri();
//  message += "\nMethod: ";
//  message += (server.method() == HTTP_GET) ? "GET" : "POST";
//  message += "\nArguments: ";
//  message += server.args();
//  message += "\n";
//
//  for (uint8_t i = 0; i < server.args(); i++) {
//    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
//  }
//
//  server.send(404, "text/plain", message);
//  digitalWrite(led, 0);
//}

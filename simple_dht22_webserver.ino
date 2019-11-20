#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DHT.h>

#define DHTPIN 0 // GPIO0, AKA "D3" on the D1 mini
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

ESP8266WebServer server(80);


void handleRoot() {
  digitalWrite(LED_BUILTIN, HIGH);
  float h, t;
  h = dht.readHumidity(); // Read humidity (percent)
  t = dht.readTemperature(); // Read temperature as C
  // Check if any reads failed and exit early (to try again).
  Serial.print("temp:");
  Serial.println(t);
  Serial.print("humi:");
  Serial.println(h);
  if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
  }

  char response[100];
  sprintf(response, "hello from esp8266. h=%f t=%f", h, t);
  server.send(200, "text/plain", response);
  digitalWrite(LED_BUILTIN, LOW);
}

void handleNotFound() {
  digitalWrite(LED_BUILTIN, HIGH);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(LED_BUILTIN, LOW);
}

void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  dht.begin();
  Serial.println("Initialized. Waiting for WiFi");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to network ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.onNotFound([]() {
    server.send(404, "text/plain", "Not Found");
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}

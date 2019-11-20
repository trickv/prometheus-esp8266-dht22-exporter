#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

#define DHTPIN 0 // GPIO0, AKA "D3" on the D1 mini
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// FIXME:
const char* device_type = "esp8266";
const char* device_id = ESP.getChipId();

ESP8266WebServer server(80);

void handleRoot() {
  digitalWrite(LED_BUILTIN, HIGH);
  float relative_humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  if (isnan(relative_humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor.");
  }
  Serial.print("Received metrics request. ");
  Serial.print("temperature: ");
  Serial.print(temperature);
  Serial.print(", relative humidity:");
  Serial.println(relative_humidity);

  String response;
  response += "# HELP temperature_c Calculated temperature in centigrade\n";
  response += "# TYPE temperature_c gauge\n";
  response += "temperature_c{device=\"" + String(device_name) + "\"} " + String(temperature);
  response += "\n";

  response += "# HELP relative_humidity Relative Humidity (RH%)\n";
  response += "# TYPE relative_humidity gauge\n";
  response += "relative_humidity{device=\"" + String(device_name) + "\"} " + String(relative_humidity);
  response += "\n";

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

  server.on("/", handleRoot);

  server.onNotFound([]() {
    server.send(404, "text/plain", "Not Found");
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}

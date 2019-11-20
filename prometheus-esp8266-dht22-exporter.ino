#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

#define DHTPIN 0 // GPIO0, AKA "D3" on the D1 mini
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

String device_id;

ESP8266WebServer server(80);

void handleRoot() {
  digitalWrite(LED_BUILTIN, HIGH);
  float relative_humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  if (isnan(relative_humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor.");
  }
  Serial.print("Received metrics request. temperature: ");
  Serial.print(temperature);
  Serial.print(", relative humidity:");
  Serial.println(relative_humidity);

  String response;
  response += "# HELP temperature_c Calculated temperature in centigrade\n";
  response += "# TYPE temperature_c gauge\n";
  response += "temperature_c{device=\"" + String(device_id) + "\"} " + String(temperature);
  response += "\n";

  response += "# HELP relative_humidity Relative Humidity (RH%)\n";
  response += "# TYPE relative_humidity gauge\n";
  response += "relative_humidity{device=\"" + String(device_id) + "\"} " + String(relative_humidity);
  response += "\n";

  response += "# HELP wifi_rssi_dbm Received Signal Strength Indication, dBm\n";
  response += "# TYPE wifi_rssi_dbm counter\n";
  response += "wifi_rssi_dbm{} %d\n\n", WiFi.RSSI();

  response += "# HELP heap_free_bytes Free heap in bytes\n";
  response += "# TYPE heap_free_bytes gauge\n";
  response += "heap_free_bytes{} %d\n\n", ESP.getFreeHeap();

  response += "# HELP esp8266_build_info System informational metric with value always \n";
  response += "# TYPE esp8266_build_info gauge\n";
  response += "esp8266_build_info{sketch_md5=\"" + String(ESP.getSketchMD5()) + "\", device_id=\"" + device_id + "\"} 1";

  server.send(200, "text/plain", response);
  digitalWrite(LED_BUILTIN, LOW);
}

void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  Serial.print("Initialized.\n\nConnecting to WiFi SSID ");
  Serial.print(WiFi.SSID());
  Serial.println();

  dht.begin();
  char device_id_char[16];
  sprintf(device_id_char, "ESP_%06X", ESP.getChipId());
  device_id = String(device_id_char);

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

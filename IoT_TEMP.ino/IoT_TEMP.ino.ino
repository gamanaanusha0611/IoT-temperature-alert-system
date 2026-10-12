

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <DHT.h>

// Wi-Fi credentials
const char* ssid = "moto edge";
const char* password = "Aa8121162822";

// Firebase Realtime Database URL
const char* DATABASE_URL =
  "https://esptemp-3d6c4-default-rtdb.firebaseio.com";

// DHT11 sensor configuration
#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  delay(1000);

  dht.begin();

  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWi-Fi connection failed!");
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi disconnected. Reconnecting...");
    WiFi.reconnect();
    delay(5000);
    return;
  }

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("DHT11 reading failed. Check sensor wiring.");
    delay(5000);
    return;
  }

  Serial.println("------------------------");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  WiFiClientSecure client;

  // Temporary testing only; skips certificate verification.
  // Use certificate validation for production.
  client.setInsecure();

  HTTPClient http;
  String url = String(DATABASE_URL) + "/temperature.json";

  if (http.begin(client, url)) {
    http.addHeader("Content-Type", "application/json");

    String payload = "{";
    payload += "\"temperature\":" + String(temperature, 1) + ",";
    payload += "\"humidity\":" + String(humidity, 1) + ",";
    payload += "\"timestamp\":{\".sv\":\"timestamp\"}";
    payload += "}";

    int responseCode = http.PUT(payload);

    Serial.print("Firebase response code: ");
    Serial.println(responseCode);

    if (responseCode >= 200 && responseCode < 300) {
      Serial.println("Firebase upload successful!");
    } else if (responseCode > 0) {
      Serial.print("Firebase error response: ");
      Serial.println(http.getString());
    } else {
      Serial.print("HTTP error: ");
      Serial.println(http.errorToString(responseCode));
    }

    http.end();
  } else {
    Serial.println("Unable to start Firebase HTTPS request.");
  }

  delay(5000);
}

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

// WIFI
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// AWS IoT
const char* aws_endpoint = "a1ai5tyxetoyg6-ats.iot.us-east-1.amazonaws.com";  // Ganti region
const int port = 8883;
const char* thingName = "DEMO_PROJECT_LKS";

// Sertifikat dari AWS (gunakan format .pem dalam PROGMEM)
const char* ca_cert = \
"-----BEGIN CERTIFICATE-----\n" \
"isidenganCA1\n" \
"-----END CERTIFICATE-----\n";

const char* client_cert = \
"-----BEGIN CERTIFICATE-----\n" \
"isidengancert\n" \
"-----END CERTIFICATE-----\n";

const char* private_key = \
"-----BEGIN RSA PRIVATE KEY-----\n" \
"isidenganprivatekey\n" \
"-----END RSA PRIVATE KEY-----\n";

// WiFiClientSecure
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

// DHT
#define DHTPIN D2     // Pin DHT11
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

void connectAWS() {
  Serial.println("Menghubungkan ke AWS IoT Core...");
  while (!client.connected()) {
    if (client.connect(thingName)) {
      Serial.println("Terhubung ke AWS IoT!");
    } else {
      Serial.print("Gagal, rc=");
      Serial.print(client.state());
      Serial.println(" Coba lagi...");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Menghubungkan WiFi...");
  }
  Serial.println("WiFi Terhubung!");

  // Sertifikat
  net.setCACert(ca_cert);
  net.setCertificate(client_cert);
  net.setPrivateKey(private_key);

  client.setServer(aws_endpoint, port);
  connectAWS();
}

void loop() {
  if (!client.connected()) {
    connectAWS();
  }
  client.loop();

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Gagal membaca sensor!");
    return;
  }

  // Buat JSON payload
  StaticJsonDocument<200> doc;
  doc["temperature"] = t;
  doc["humidity"] = h;

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);

  Serial.println("Kirim Data: ");
  Serial.println(jsonBuffer);

  client.publish("esp8266/dht11", jsonBuffer);
  delay(5000);
}

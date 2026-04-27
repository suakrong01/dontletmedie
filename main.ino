#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DHTPIN 4
#define DHTTYPE DHT11
#define SOIL_PIN 34
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define WIFI_SSID "IOT"
#define WIFI_PASSWORD "mfuiot2023"
#define DATABASE_URL "https://dontletmedie-default-rtdb.asia-southeast1.firebasedatabase.app"
#define API_KEY "AIzaSyAEDiu-Y-SWh_oX6jNo-45JiH3Vk5VPS_0"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

unsigned long sendDataPrevMillis = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  dht.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Connecting...");
  display.display();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("กำลังเชื่อม WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi OK!");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  Firebase.signUp(&config, &auth, "", "");
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Connected!");
  display.display();
}

void loop() {
  if (millis() - sendDataPrevMillis > 2000) {
    sendDataPrevMillis = millis();

    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    int soilRaw = analogRead(SOIL_PIN);
    int soilPercent = map(soilRaw, 4095, 0, 0, 100);

    if (Firebase.ready()) {
      Firebase.RTDB.setFloat(&fbdo, "/plant/soil", soilPercent);
      Firebase.RTDB.setFloat(&fbdo, "/plant/humidity", humidity);
      Firebase.RTDB.setFloat(&fbdo, "/plant/temperature", temperature);
      Serial.print("Soil: "); Serial.print(soilPercent);
      Serial.print("% Humid: "); Serial.print(humidity);
      Serial.print("% Temp: "); Serial.println(temperature);
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println("** Smart Plant **");
    display.drawLine(0, 10, 128, 10, WHITE);
    display.setCursor(0, 16);
    display.print("Soil : ");
    display.print(soilPercent);
    display.println(" %");
    display.setCursor(0, 30);
    display.print("Humid: ");
    display.print(humidity);
    display.println(" %");
    display.setCursor(0, 44);
    display.print("Temp : ");
    display.print(temperature);
    display.println(" C");
    if (soilPercent < 30) {
      display.setCursor(0, 56);
      display.println("!! Water needed !!");
    }
    display.display();
  }
}

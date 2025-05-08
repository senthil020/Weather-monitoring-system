#define BLYNK_TEMPLATE_ID "TMPL37Y9qVAjs"
#define BLYNK_TEMPLATE_NAME "Weather monitoring system"
#define BLYNK_AUTH_TOKEN "fR-iiykkwChE6r3LKmQUUizASCZAQKeg"

#define BLYNK_PRINT Serial

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <SFE_BMP180.h>

#define LDR 4
#define TH 5
#define Rain 36

double T, P;
char status;

LiquidCrystal_I2C lcd(0x27, 16, 2);
SFE_BMP180 bmp;
DHT dht(TH, DHT11);
BlynkTimer timer;
WidgetLED LED(V4); // Blynk LED widget

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "senthil";
char pass[] = "1234567890";

// Auto reconnect function
void checkConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected! Reconnecting...");
    WiFi.begin(ssid, pass);
  }

  if (!Blynk.connected()) {
    Serial.println("Blynk disconnected! Reconnecting...");
    Blynk.connect();
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Starting...");

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");

  // Connect to Blynk Cloud (IoT server)
  Blynk.config(auth, "blynk.cloud", 80);
  Blynk.connect();

  bmp.begin();
  dht.begin();

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("System");
  lcd.setCursor(4, 1);
  lcd.print("Loading..");
  delay(4000);
  lcd.clear();

  pinMode(LDR, INPUT);
  pinMode(Rain, INPUT);
  analogReadResolution(12);

  timer.setInterval(2000L, DHT11sensor);
  timer.setInterval(2500L, rainSensor);
  timer.setInterval(3000L, pressureSensor);
  timer.setInterval(1500L, LDRsensor);

  // Check WiFi and Blynk every 10 seconds
  timer.setInterval(10000L, checkConnection);
}

void DHT11sensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);

  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(t);
  lcd.print(" ");

  lcd.setCursor(8, 0);
  lcd.print("H:");
  lcd.print(h);
  lcd.print(" ");

  Serial.print("Temp: ");
  Serial.print(t);
  Serial.print(" °C | Humidity: ");
  Serial.print(h);
  Serial.println(" %");
}

void rainSensor() {
  int Rvalue = analogRead(Rain);
  Rvalue = map(Rvalue, 0, 4095, 0, 100);
  Rvalue = (Rvalue - 100) * -1;

  Blynk.virtualWrite(V2, Rvalue);

  lcd.setCursor(0, 1);
  lcd.print("R:");
  lcd.print(Rvalue);
  lcd.print(" ");

  Serial.print("Rain: ");
  Serial.print(Rvalue);
  Serial.println(" %");
}

void pressureSensor() {
  status = bmp.startTemperature();
  if (status != 0) {
    delay(status);
    status = bmp.getTemperature(T);
    status = bmp.startPressure(3);
    if (status != 0) {
      delay(status);
      status = bmp.getPressure(P, T);
      if (status != 0) {
        Blynk.virtualWrite(V3, P);
        lcd.setCursor(8, 1);
        lcd.print("P:");
        lcd.print(P);
        lcd.print(" ");

        Serial.print("Pressure: ");
        Serial.print(P);
        Serial.println(" Pa");
      }
    }
  } else {
    Serial.println("BMP180 sensor read failed");
  }
}

void LDRsensor() {
  bool value = digitalRead(LDR);
  if (value == 1) {
    LED.on();
    Serial.println("LDR: Bright");
  } else {
    LED.off();
    Serial.println("LDR: Dark");
  }
}

void loop() {
  if (Blynk.connected()) {
    Blynk.run();
  }
  timer.run();
}

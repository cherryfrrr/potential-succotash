#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define BUZZER_PIN 8
#define BUTTON_PIN 7

// Unsafe thresholds
#define TEMP_HIGH 34.0      // °C
#define TEMP_LOW 0.0        // °C
#define HUMIDITY_HIGH 70.0  // %
#define HUMIDITY_LOW 20.0   // %

bool alarmSilenced = false;
bool lastButtonState = HIGH;

void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(BUTTON_PIN, INPUT_PULLUP);  

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 not found"));
    while (1)
      ;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 5);
  display.println("ThermoGuard R3");
  display.display();
  delay(2000);
}

void loop() {
  float humidity = dht.readHumidity();
  float temperatureC = dht.readTemperature();

  if (isnan(humidity) || isnan(temperatureC)) {
    Serial.println(F("DHT read failed!"));
    return;
  }

  //toggle
  bool currentButtonState = digitalRead(BUTTON_PIN);
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    alarmSilenced = !alarmSilenced;
    Serial.println(alarmSilenced ? "Alarm silenced!" : "Alarm enabled!");
    delay(50);
  }
  lastButtonState = currentButtonState;

  //check conditions
  bool tempUnsafe = (temperatureC > TEMP_HIGH || temperatureC < TEMP_LOW);
  bool humidityUnsafe = (humidity > HUMIDITY_HIGH || humidity < HUMIDITY_LOW);
  bool dangerous = (tempUnsafe || humidityUnsafe);

  //buzzer
  if (dangerous && !alarmSilenced) {
    digitalWrite(BUZZER_PIN, HIGH);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }

  //display
  display.clearDisplay();

  if (dangerous) {
    display.invertDisplay(true);
  } else {
    display.invertDisplay(false);
  }

  display.setTextSize(1);
  display.setCursor(0, 5);
  display.println("ThermoGuard R3");

  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print(temperatureC, 1);
  display.println(" C");

  display.setTextSize(1);
  display.setCursor(0, 30);
  display.print("Humidity: ");
  display.print(humidity, 0);
  display.println("%");

  if (alarmSilenced) {
    display.setCursor(90, 5);
    display.println("MUTE");
  }

  //show danger
  if (dangerous) {
    display.setCursor(0, 40);
    if (tempUnsafe && temperatureC > TEMP_HIGH) display.println("TEMP HIGH!");
    if (tempUnsafe && temperatureC < TEMP_LOW) display.println("TEMP LOW!");
    display.setCursor(0, 50);
    if (humidityUnsafe && humidity > HUMIDITY_HIGH) display.println("HUMID HIGH!");
    if (humidityUnsafe && humidity < HUMIDITY_LOW) display.println("HUMID LOW!");
  }

  display.display();
  delay(100);
}
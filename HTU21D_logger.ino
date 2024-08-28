#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <Wire.h>

// HTU21D sensor library
#include "Adafruit_HTU21DF.h"

// DS3231 RTC library
#include "RTClib.h"

// OLED display libraries
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define OLED display parameters
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Define SPI pins for ESP32
#define SCK 18
#define MISO 19
#define MOSI 23
#define CS 5

Adafruit_HTU21DF htu = Adafruit_HTU21DF();
RTC_DS3231 rtc;

void setup() {
  // Initialize Serial communication
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;);
  }
  display.display();
  delay(100);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Initialize HTU21D sensor
  if (!htu.begin()) {
    Serial.println("Couldn't find HTU21D sensor!");
    while (1);
  }

  // Initialize DS3231 RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC!");
    while (1);
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Set RTC to the date & time this sketch was compiled
  }

  // Initialize SPI and SD card
  SPI.begin(SCK, MISO, MOSI, CS);
  if (!SD.begin(CS)) {
    Serial.println("Card Mount Failed");
    return;
  }

  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
}

void loop() {
  float temp = htu.readTemperature();
  float hum = htu.readHumidity();
  DateTime now = rtc.now();


  // Open file for writing
  File myfile = SD.open("/Readings.csv", FILE_APPEND);
  if (!myfile) {
    Serial.println("Failed to open file for writing");
    return;
  }

  // Write CSV header if file is new
  if (myfile.size() == 0) {
    myfile.println("Date,Time,Temperature (°C),Humidity (%)");
  }

  // Write data to the file and Serial monitor
  myfile.printf("%04d/%02d/%02d,%02d:%02d:%02d,%.2f,%.2f\n",
                now.year(), now.month(), now.day(),
                now.hour(), now.minute(), now.second(),
                temp, hum);

  Serial.printf("%04d/%02d/%02d %02d:%02d:%02d - Temperature(°C): %.2f, Humidity: %.2f%%\n",
                now.year(), now.month(), now.day(),
                now.hour(), now.minute(), now.second(),
                temp, hum);

  // Close the file
  myfile.close();

  // Display on OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.printf("%04d/%02d/%02d %02d:%02d:%02d",
                 now.year(), now.month(), now.day(),
                 now.hour(), now.minute(), now.second());

  display.setCursor(0, 15);
  display.print("Temp: ");
  display.setTextSize(1.5);
  display.setCursor(0, 25);
  display.print(temp);
  display.cp437(true);
  display.write(167); // Degree symbol
  display.print("C");

  display.setTextSize(1);
  display.setCursor(0, 45);
  display.print("Humidity: ");
  display.setTextSize(1.5);
  display.setCursor(0, 55);
  display.print(hum);
  display.print(" %");

  display.display();

  delay(5000);
}

#include <WiFi.h>
#include <ThingSpeak.h>
#include "DHT.h"

// Konfigurasi Wi-Fi
const char* ssid = "INI WIFI"; 
const char* password = "***";

// Konfigurasi ThingSpeak
WiFiClient client;
unsigned long myChannelNumber = 0; // Channel ID (opsional)
const char * myWriteAPIKey = "6UL1983GJUZHKIWW";

// Pin & Tipe Sensor DHT22
#define DHTPIN 4     
#define DHTTYPE DHT22   
DHT dht(DHTPIN, DHTTYPE);

// Pin Sensor MQ-2 Analog
#define MQ2PIN 34    // GPIO 34 (ADC1)

void setup() {
  Serial.begin(115200);
  Serial.println(F("Pengujian DHT22 + MQ-2 + ThingSpeak pada ESP32"));

  // Inisialisasi sensor DHT
  dht.begin();

  // Koneksi ke Wi-Fi
  WiFi.mode(WIFI_STA);
  Serial.print("Menghubungkan ke WiFi ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nTerhubung ke WiFi!");

  // Inisialisasi ThingSpeak
  ThingSpeak.begin(client);
}

void loop() {
  // ThingSpeak memerlukan jeda minimal 15 detik antar pengiriman data
  delay(15000);

  // 1. Membaca Sensor DHT22
  float kelembapan = dht.readHumidity();
  float suhu = dht.readTemperature();

  // 2. Membaca Sensor MQ-2 (ADC ESP32 beresolusi 0-4095)
  int nilaiGas = analogRead(MQ2PIN);

  // Cek jika pembacaan DHT22 gagal
  if (isnan(kelembapan) || isnan(suhu)) {
    Serial.println(F("Gagal membaca dari sensor DHT22! Periksa kabel rangkaian."));
    return;
  }

  // Tampilkan data pada Serial Monitor
  Serial.print(F("Suhu: "));
  Serial.print(suhu);
  Serial.print(F(" °C | Kelembapan: "));
  Serial.print(kelembapan);
  Serial.print(F(" % | Nilai Gas/Asap (MQ-2): "));
  Serial.println(nilaiGas);

  // Set nilai Field pada ThingSpeak
  ThingSpeak.setField(1, suhu);        // Field 1: Suhu
  ThingSpeak.setField(2, kelembapan);  // Field 2: Kelembapan
  ThingSpeak.setField(3, nilaiGas);    // Field 3: MQ-2 Gas Value

  // Kirim data ke ThingSpeak
  int httpCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  if (httpCode == 200) {
    Serial.println("Data berhasil dikirim ke ThingSpeak!");
  } else {
    Serial.println("Gagal mengirim data. Kode Error HTTP: " + String(httpCode));
  }
}

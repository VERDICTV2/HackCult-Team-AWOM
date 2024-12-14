#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <DHT.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

// DHT22 setup
#define DHTPIN 4  // GPIO connected to the DHT22
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// BMP280 setup
#define BMP280_I2C_ADDRESS 0x76 // Default I2C address for BMP280
Adafruit_BMP280 bmp; // Use I2C interface

// GPS setup
HardwareSerial gpsSerial(2); // Use Serial2 for GPS communication
TinyGPSPlus gps;

// Configure Firebase
#define FIREBASE_HOST "awom-c91d6-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "AIzaSyBg0_jYFfOzGHVyiGUJ-h1NfDHScXKNpoo" // Replace with your Web API key
// Wi-Fi credentials
#define WIFI_SSID "SIT-Guest"
#define WIFI_PASSWORD "tp151024"

FirebaseData firebaseData;
FirebaseJson json;
FirebaseConfig firebaseConfig;
FirebaseAuth firebaseAuth;

// Buzzer setup
#define BUZZER_PIN 5  // GPIO connected to the buzzer

void setup() {
  Serial.begin(115200);

  // Buzzer initialization
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // DHT22 initialization
  dht.begin();

  // BMP280 initialization
  if (!bmp.begin(BMP280_I2C_ADDRESS)) {
    Serial.println("Could not find BMP280 sensor!");
    while (1); // Stay here if BMP280 initialization fails
  }
  Serial.println("BMP280 initialized!");

  // GPS initialization
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17); // RX2 (GPIO16), TX2 (GPIO17)

  // Wi-Fi connection
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected!");

  Serial.println("Firebase connected!");
  Serial.println("Setup completed!");
}

void loop() {
  // Read pressure and temperature from BMP280
  float pressureHpa = bmp.readPressure() / 100.0; // Pressure in hPa
  float temperatureBMP = bmp.readTemperature(); // Temperature in Celsius

  // Read DHT22 data
  float temperatureDHT = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Check if DHT22 data is valid
  if (isnan(temperatureDHT) || isnan(humidity)) {
    Serial.println("Failed to read from DHT22!");
  }

  // Read GPS data
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read(); // Read one character
    if (gps.encode(c)) { // If valid data is parsed
      displayGPSData();
    }
  }

  // Check temperature condition for buzzer
  if (temperatureBMP > 30.0 || temperatureDHT > 30.0) {
    digitalWrite(BUZZER_PIN, HIGH); // Turn on the buzzer
    Serial.println("Temperature exceeded 30°C! Buzzer activated.");
  } else {
    digitalWrite(BUZZER_PIN, LOW); // Turn off the buzzer
  }

  // Print sensor readings
  Serial.println("--- Sensor Data ---");
  Serial.print("Pressure (BMP280): ");
  Serial.print(pressureHpa);
  Serial.println(" hPa");
  json.set("/Pressure", pressureHpa);

  Serial.print("BMP280 Temperature: ");
  Serial.print(temperatureBMP);
  Serial.println(" °C");
  json.set("/BMP280 Temperature", temperatureBMP);

  if (!isnan(temperatureDHT) && !isnan(humidity)) {
    Serial.print("DHT22 Temperature: ");
    Serial.print(temperatureDHT);
    Serial.println(" °C");
    json.set("/DHT22 Temperature", temperatureDHT);

    Serial.print("DHT22 Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    json.set("/DHT22 Humidity", humidity);
  }

  // Delay for readability
  delay(2000);
}

void displayGPSData() {
  // Display latitude and longitude if available
  if (gps.location.isValid()) {
    Serial.print("Latitude: ");
    Serial.println(gps.location.lat(), 6);
    json.set("/Latitude", gps.location.lat());
    Serial.print("Longitude: ");
    Serial.println(gps.location.lng(), 6);
    json.set("/Longitude", gps.location.lng());
  } else {
    Serial.println("Location: Not available");
  }

  // Display date and time if available
  if (gps.date.isValid() && gps.time.isValid()) {
    Serial.print("Date: ");
    Serial.print(gps.date.month());
    Serial.print("/");
    Serial.print(gps.date.day());
    Serial.print("/");
    Serial.println(gps.date.year());

    Serial.print("Time: ");
    Serial.print(gps.time.hour());
    Serial.print(":");
    Serial.print(gps.time.minute());
    Serial.print(":");
    Serial.println(gps.time.second());
  } else {
    Serial.println("Date/Time: Not available");
  }

  // Display the number of satellites
  if (gps.satellites.isValid()) {
    Serial.print("Satellites: ");
    Serial.println(gps.satellites.value());
    json.set("/Satellites", gps.satellites.value());
  } else {
    Serial.println("Satellites: Not available");
  }

  // Display altitude if available
  if (gps.altitude.isValid()) {
    Serial.print("Altitude: ");
    Serial.print(gps.altitude.meters());
    json.set("/Altitude", gps.altitude.meters());
    Serial.println(" meters");
  } else {
    Serial.println("Altitude: Not available");
  }

  Serial.println();

  // Update Firebase
  if (Firebase.updateNode(firebaseData, "/Tracker", json)) {
    Serial.println("Data successfully sent to Firebase!");
  } else {
    Serial.print("Failed to send data to Firebase. Error: ");
    Serial.println(firebaseData.errorReason());
  }
}

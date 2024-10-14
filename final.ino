#include <ESP32Servo.h>
#include <WiFi.h>
#include <DHT_U.h>
#include <DHT.h>
#include <Wire.h>
#include <LCD_I2C.h>
#include <Keypad.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#define DHTTYPE DHT11  // DHT 11 (AM2302)
#define DHTPIN 32      // what pin we're connected to
#define SENS 34        // Adjust this pin according to your setup
#define pumpPin 26
#define SERVO 25
#define RON LOW
#define ROF HIGH
DHT dht(DHTPIN, DHTTYPE);
LCD_I2C lcd(0x27, 16, 2);   // Set the LCD I2C address and dimensions (16x2)
LCD_I2C lcd1(0x26, 16, 2);  // Set the LCD I2C address and dimensions (16x2)

const char* ssid = "Abdullah Wifi";   // Change to your WiFi SSID
const char* password = "asdqwe1234";  // Change to your WiFi password

WiFiServer server(80);
const int Password_Length = 7;
String Data;
String Master = "1133779";
Servo servo;
bool flSt = false;
int i = 0;
bool up = false;
int lockOutput = 33;
int led1 = 13;
int led2 = 12;
int led3 = 14;
int led4 = 27;
byte data_count = 0;
char customKey;
const byte ROWS = 4;
const byte COLS = 4;
const int motorPin1 = 25;
char hexaKeys[ROWS][COLS] = {
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' },
  { '*', '0', '#' }
};
byte rowPins[ROWS] = { 23, 19, 18, 5 };
byte colPins[COLS] = { 4, 2, 15 };
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
int attempts = 0;
const int maxAttempts = 3;

#define API_KEY "AIzaSyAzJfoKQI7tcuDu6rvum8dWlfkcDbU7MZk"                    // Change to your Firebase API key
#define DATABASE_URL "https://smarthome-58ae1-default-rtdb.firebaseio.com/"  // Change to your Firebase database URL

FirebaseData fbdo, fbdo_s1, fbdo_s2, fbdo_s3, fbdo_s4, fbdo_s5, fbdo_s6;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;  //since we are doing an anonymous sign in

bool Homelightred;
bool Homelightyellow;
float h = dht.readHumidity();
float t = dht.readTemperature();

void setup() {
  Serial.begin(115200);
  delay(1000);  // Wait for serial monitor to initialize

  pinMode(lockOutput, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(motorPin1, OUTPUT);
  dht.begin();
  lcd.begin();  // If you are using more I2C devices using the Wire library use lcd.begin(false)
  lcd.backlight();
  lcd1.begin();  // If you are using more I2C devices using the Wire library use lcd.begin(false)
  lcd1.backlight();
  digitalWrite(lockOutput, 0);
  pinMode(SENS, INPUT_PULLUP);
  pinMode(pumpPin, OUTPUT);

  servo.attach(SERVO);
  servo.write(0);

  digitalWrite(led1, 0);
  digitalWrite(led2, 0);
  digitalWrite(led3, 0);
  digitalWrite(led4, 0);
  digitalWrite(pumpPin, 0);
  digitalWrite(motorPin1, 0);

  Serial.println();
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  server.begin();

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase signup successful.");
    signupOK = true;
  } else {
    Serial.println("Firebase signup failed.");
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  if (!Firebase.RTDB.beginStream(&fbdo_s1, "Homelightred/Switch"))
    Serial.printf("stream 1 begin error, %s\n\n", fbdo_s1.errorReason().c_str());
  if (!Firebase.RTDB.beginStream(&fbdo_s2, "Homelightyellow/Switch"))
    Serial.printf("stream 2 begin error, %s\n\n", fbdo_s2.errorReason().c_str());

  if (!Firebase.RTDB.beginStream(&fbdo_s3, "Gardenlight/Switch"))
    Serial.printf("stream 3 begin error, %s\n\n", fbdo_s3.errorReason().c_str());
  if (!Firebase.RTDB.beginStream(&fbdo_s4, "GarageLight/Switch"))
    Serial.printf("stream 4 begin error, %s\n\n", fbdo_s4.errorReason().c_str());

  if (!Firebase.RTDB.beginStream(&fbdo_s5, "HomeDoor/Switch"))
    Serial.printf("stream 5 begin error, %s\n\n", fbdo_s5.errorReason().c_str());
  if (!Firebase.RTDB.beginStream(&fbdo_s6, "FanState/Switch"))
    Serial.printf("stream 6 begin error, %s\n\n", fbdo_s6.errorReason().c_str());
}

void loop() {
  Access();
  Temp();
  WIFIRead();
  MoileApp();
  /*
  if (Firebase.RTDB.setFloat(&fbdo, "DHT11part1/Temp", t) && Firebase.RTDB.setFloat(&fbdo, "DHT11part2/Humidity", h)) {
    Serial.print("Temperature  ");
  } else {
    Serial.println("Failed: " + fbdo.errorReason());
  }
  */
}

void WIFIRead() {
  WiFiClient client = server.available();

  if (client) {
    Serial.println("New client connected.");
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.print("Click <a href=\"/H1\">here</a> to turn LED 1 on.<br>");
            client.print("Click <a href=\"/L1\">here</a> to turn LED 1 off.<br>");
            client.print("Click <a href=\"/H2\">here</a> to turn LED 2 on.<br>");
            client.print("Click <a href=\"/L2\">here</a> to turn LED 2 off.<br>");
            client.print("Click <a href=\"/H3\">here</a> to turn LED 3 on.<br>");
            client.print("Click <a href=\"/L3\">here</a> to turn LED 3 off.<br>");
            client.print("Click <a href=\"/H4\">here</a> to turn LED 4 on.<br>");
            client.print("Click <a href=\"/L4\">here</a> to turn LED 4 off.<br>");
            client.print("Click <a href=\"/P\">here</a> to turn Pump on.<br>");
            client.print("Click <a href=\"/PT\">here</a> to turn Pump off.<br>");
            client.print("Click <a href=\"/M\">here</a> to turn Motor on.<br>");
            client.print("Click <a href=\"/MT\">here</a> to turn Motor off.<br>");
            client.print("Click <a href=\"/LO\">here</a> to unlock.<br>");
            client.print("Click <a href=\"/LCT\">here</a> to lock.<br>");
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }

        if (currentLine.endsWith("GET /H1")) {
          digitalWrite(led1, 1);
          SetMobileLed1(true);
        }
        if (currentLine.endsWith("GET /L1")) {
          digitalWrite(led1, 0);
          SetMobileLed1(false);
        }
        if (currentLine.endsWith("GET /H2")) {
          digitalWrite(led2, 1);
          SetMobileLed2(true);
        }
        if (currentLine.endsWith("GET /L2")) {
          digitalWrite(led2, 0);
          SetMobileLed2(false);
        }
        if (currentLine.endsWith("GET /H3")) {
          digitalWrite(led3, 1);
          SetMobileLed3(true);
        }
        if (currentLine.endsWith("GET /L3")) {
          digitalWrite(led3, 0);
          SetMobileLed3(false);
        }
        if (currentLine.endsWith("GET /H4")) {
          digitalWrite(led4, 1);
          SetMobileLed4(true);
        }
        if (currentLine.endsWith("GET /L4")) {
          digitalWrite(led4, 0);
          SetMobileLed4(false);
        }
        if (currentLine.endsWith("GET /P")) {
          digitalWrite(pumpPin, HIGH);  // Turn on pump
        }
        if (currentLine.endsWith("GET /PT")) {
          digitalWrite(pumpPin, LOW);  // Turn off pump
        }
        if (currentLine.endsWith("GET /M")) {
          digitalWrite(motorPin1, 1);  // Turn on motor
          SetMobileFan(true);
        }
        if (currentLine.endsWith("GET /MT")) {
          digitalWrite(motorPin1, 0);  // Turn off motor
          SetMobileFan(false);
        }
        if (currentLine.endsWith("GET /LO")) {
          Face();
          SetMobileLock(true);
        }
        if (currentLine.endsWith("GET /LCT")) {
          digitalWrite(lockOutput, 0);
          SetMobileLock(false);
          delay(250);  // Lock
        }
      }
      // Add more control actions for other devices as needed
    }
  }
  client.stop();
  Serial.println("Client disconnected.");
}
void Temp() {
  if (t > 16 && t < 33) {
    stopMotor();
    lcd.setCursor(0, 0);
    lcd.println(" Now Temperature ");
    lcd.setCursor(0, 1);
    lcd.print("T:");
    lcd.print(t);
    lcd.print("C");
    lcd.setCursor(6, 1);
    lcd.println("&&  ");
    lcd.setCursor(9, 1);
    lcd.print("H:");
    lcd.print(h);
    lcd.print("%");
  } else if (t > 33 && t < 35) {
    lcd.setCursor(0, 0);
    lcd.println("Temp is high");
    lcd.setCursor(0, 1);
    lcd.println("open the fans   ");
  } else if (t >= 35) {
    forward();
    lcd.setCursor(0, 0);
    lcd.println("Air conditioners ");
    lcd.setCursor(0, 1);
    lcd.println("are running  ");
  } else if (t <= 16 && t > 0) {
    lcd.setCursor(0, 0);
    lcd.println("Temp is very low ");
    lcd.setCursor(0, 1);
    lcd.println("open the Heaters ");
  } else {
    lcd.setCursor(0, 0);

    lcd.println("There is no reading");
    lcd.setCursor(0, 1);
    lcd.println("Check The Sensor ");
  }
}

void Access() {
  lcd1.setCursor(0, 0);
  lcd1.print("Enter Password:");

  customKey = customKeypad.getKey();
  if (customKey) {
    Data += customKey;
    lcd1.setCursor(data_count, 1);
    lcd1.print("*");
    data_count++;
  }

  if (data_count == Password_Length) {
    lcd1.clear();
    if (Data == Master) {
      Face();
    } else {
      lcd1.setCursor(0, 0);
      lcd1.print("Incorrect");
      attempts++;
      delay(1000);
      lcd1.clear();
    }
    clearData();
    if (attempts >= 2) {
      lcd1.clear();
      lcd1.setCursor(0, 0);
      lcd1.print("Max Attempts");
      delay(6000);
      lcd1.clear();
      attempts = 0;
    }
  }
}

void clearData() {
  data_count = 0;
  Data = "";
}

void Face() {
  lcd1.print("Enter success");
  digitalWrite(lockOutput, 1);
  delay(3000);
  digitalWrite(lockOutput, 0);
  delay(250);
}

void MoileApp() {


  if (Firebase.ready() && signupOK) {
    if (!Firebase.RTDB.readStream(&fbdo_s1))
      Serial.printf("stream 1 read error, %s\n\n", fbdo_s1.errorReason().c_str());
    if (fbdo_s1.streamAvailable()) {
      Homelightred = fbdo_s1.boolData();
      digitalWrite(led1, Homelightred);
    }

    if (!Firebase.RTDB.readStream(&fbdo_s2))
      Serial.printf("stream 2 read error, %s\n\n", fbdo_s2.errorReason().c_str());
    if (fbdo_s2.streamAvailable()) {
      Homelightyellow = fbdo_s2.boolData();
      digitalWrite(led2, Homelightyellow);
    }
    if (!Firebase.RTDB.readStream(&fbdo_s3))
      Serial.printf("stream 3 read error, %s\n\n", fbdo_s3.errorReason().c_str());
    if (fbdo_s3.streamAvailable()) {
      bool Gardenlight = fbdo_s3.boolData();
      digitalWrite(led3, Gardenlight);
    }

    if (!Firebase.RTDB.readStream(&fbdo_s4))
      Serial.printf("stream 4 read error, %s\n\n", fbdo_s4.errorReason().c_str());
    if (fbdo_s4.streamAvailable()) {
      bool GarageLight = fbdo_s4.boolData();
      digitalWrite(led4, GarageLight);
    }
    if (!Firebase.RTDB.readStream(&fbdo_s5))
      Serial.printf("stream 5 read error, %s\n\n", fbdo_s5.errorReason().c_str());
    if (fbdo_s5.streamAvailable()) {
      bool HomeDoor = fbdo_s5.boolData();
      digitalWrite(lockOutput, HomeDoor);
    }

    if (!Firebase.RTDB.readStream(&fbdo_s6))
      Serial.printf("stream 6 read error, %s\n\n", fbdo_s6.errorReason().c_str());
    if (fbdo_s6.streamAvailable()) {
      bool FanState = fbdo_s6.boolData();
      digitalWrite(motorPin1, FanState);
    }
  }
}

void SetMobileLed1(bool L) {
  if (Firebase.RTDB.setBool(&fbdo, "Homelightred/Switch", L)) {
    Serial.println("Led 1 state set in Firebase.");
  } else {
    Serial.println("Failed to set led 1 state in Firebase.");
  }
}

void forward() {
  digitalWrite(motorPin1, 1);
}

void stopMotor() {
  digitalWrite(motorPin1, 0);
}

void SetMobileLed2(bool L) {
  if (Firebase.RTDB.setBool(&fbdo, "Homelightyellow/Switch", L)) {
    Serial.println("Led 2 state set in Firebase.");
  } else {
    Serial.println("Failed to set led 2 state in Firebase.");
  }
}
void SetMobileLed3(bool L) {
  if (Firebase.RTDB.setBool(&fbdo, "Gardenlight/Switch", L)) {
    Serial.println("Led 3 state set in Firebase.");
  } else {
    Serial.println("Failed to set led 3 state in Firebase.");
  }
}
void SetMobileLed4(bool L) {
  if (Firebase.RTDB.setBool(&fbdo, "GarageLight/Switch", L)) {
    Serial.println("Led 4 state set in Firebase.");
  } else {
    Serial.println("Failed to set led 4 state in Firebase.");
  }
}
void SetMobileFan(bool L) {
  if (Firebase.RTDB.setBool(&fbdo, "FanState/Switch", L)) {
    Serial.println("Fan state set in Firebase.");
  } else {
    Serial.println("Failed to set led 4 state in Firebase.");
  }
}
void SetMobileLock(bool L) {
  if (Firebase.RTDB.setBool(&fbdo, "HomeDoor/Switch", L)) {
    Serial.println("Fan state set in Firebase.");
  } else {
    Serial.println("Failed to set led 4 state in Firebase.");
  }
}
void Sens() {
  if (digitalRead(SENS)) {
    flSt = true;
  } else {
    flSt = false;
  }
}

void Pump() {
  int flameValue = digitalRead(SENS);
  if (flameValue == LOW) {
    digitalWrite(pumpPin, 1);
  } else {
    digitalWrite(pumpPin, 0);
  }
}

void Fire() {
  Sens();
  if (flSt) {
    Pump();
    delay(500);
    if (servo.attached()) {
      servo.detach();
    }
  } else {
    Pump();
    delay(500);
    if (!servo.attached())
      servo.attach(SERVO);
    else {
      if (!up) {
        do {
          Sens();
          i++;
          servo.write(i);
          if (i == 180) up = true;
          if (flSt) {
            servo.detach();
            break;
          }
          delay(5);
        } while (i <= 180 && !flSt);
      } else {
        do {
          Sens();
          i--;
          servo.write(i);
          if (i == 0) up = false;
          if (flSt) {
            servo.detach();
            break;
          }
          delay(5);
        } while (i >= 0 && !flSt);
      }
    }
  }
}

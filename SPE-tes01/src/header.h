#include <Arduino.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <HX711.h>
#include <SPI.h>
#include <WiFi.h>
#include <time.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <WiFi.h>
#include <MAX7219_7Seg_Disp.h>
#include <FirebaseESP32.h>
#include <NTPClient.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// WiFi
#define WIFI_SSID "robs"
#define WIFI_PASSWORD "d5tt2dua"

// Firebase
#define DATABASE_URL "https://esp-scale-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define API_KEY "AIzaSyA3lJ_uKI-s_QASFbfv4j-SsPjGD4Vm6aM"

#define FIREBASE_PROJECT_ID "esp-scale"
#define FIREBASE_CLIENT_EMAIL "firebase-adminsdk-62ho0@esp-scale.iam.gserviceaccount.com"
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDZvM5135im8iY+\n6dp5/jCF24R/QYNSngVN7U/rvqx4ejPGU9ozvIjtVuUvxC6GPkRBui+m63RD/RGx\nB+hWphwedpQzBwgiumNdTjZd48XpKCO6KLiVNlyxMzS3AomyCn0Km5UhA41x7t9U\nX/XVwWRZYV4XTBzktnwH1AOJoSo2ImuK4Avb+Dh7AvT6lojPxrt/kJbPlTkLywZl\nNQlhR/vgStO2tGSYDwbfmQSFylaUBY8Vxgnm/vlhI3ar9V05oetn9gMQhMXdPWWD\nhKXugtIK0/GEVPfiQJSxiLDXrC27VSwlWgeAKyAsWQbCLi9ekkqjWRSHRSREwsRj\niJETJnsRAgMBAAECggEAS6dJiTU2ghMy7LsuWSLT0z6xq9Gkgmk7aJcjJMMsg/PZ\n13g9IarJXMXLfixJCgXKi5fYhMPcst7UbdFUjf7LWF5VIeG4lvByI/JP+5Fvq5Jx\nz3XkpJut2zfyggAUvdLdHXlveHgWhla8LEHnVe0LI+8r1SZyB5nMQKAO1pwz4Xiy\nBuHEd1ojYVJgkRlcE4sDQAv5oYkZsIB5xL37VLTFxIshuzyIcc0Vrph/WWNzcizf\n6omoTYzJVTtWhPDn+tnMtxxY3t5ZN5lLXgzmskT1s6mN1GLe2K2QWPHu6Cwm8eY8\nLagx4OIkPk2+hbK07kPbBUEa38WgNSY5MlG+M8Fc3QKBgQDzXd3SJbiWNrnQbysY\nu7c/KbJqSyHK1F17pEeDVUN/ZKzswb/04NMPA8+HurzCRV9gc2x2IG9vhJ3yS63d\n/i4R3HeSuNxdABSe2KSgtsYqb2vMdMXl7FeE6NvCOpm24VMgLOd4Nc35dsggUChU\nFY2t4dLWzhk6dNlqFlSUW72FQwKBgQDlClnMsXx7BpQ+N4Hoye1j0K1Ok3ZlX4zQ\ns4HayMDV6ZF8zWDzij9hZO5DCtsRuFXGlNdkdEP62NZ3BzwkoflkO1XJYSqbRnjY\nNoN2/RP/O/1tOemI7rMNkx0cMF52G+ijXIi8Iqq1/P0qiRQSNFtgsq7bvHieyHm5\nLlrNHFOPGwKBgBjfXQ5fQJBJo5AD7Jmohb/02QE4EIoUTkGWtqBblHxuo6XG34F9\ngNSmBQ3QLf6E7IyE/27LPdhd626aAXkQw1CcAbSDJDJ1EaZyadDrHTG1FYVd4ePv\n1boFuoANUSx5tu8w569HgHeghi+XQafmVAaTb+L4SWfKDXZkNdy7y5DVAoGBANtI\naqtALja3GMzsZdMTKNWMn7CAHDV4IqBZjHSECaODyIfdNfpVHNNJR5gmV5EClBTY\nwdNqQP90BPjWk21bKiMdb8eq4JjjAqW2o20TZcbSj5K4hG9WYVUYySEI6hdYJwXo\nfPzqj9hHC7dioDXBrM7KpAC61BzpQSjbSZHgkduLAoGBAK1KwNa7PcSolpjmU3Ro\npmYX4tSJikMfbZqZPMtYp4v1ZbpmnBNWbm72Vx+CXw/BoW2URqtSUQo2oO9Tr1ng\nhJ5Tw0TMLeeVyPGSgTLqxuRPgeH6J/3WwFJI07E6z1/ts47LIh2Rp7moANclrQ1o\n79ggmuUjtwW+Gj8Rz7P1PX04\n-----END PRIVATE KEY-----\n";

SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
SemaphoreHandle_t sSemaphore = xSemaphoreCreateBinary();
SemaphoreHandle_t cSemaphore = xSemaphoreCreateCounting(4, 4);

FirebaseJson json;
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

String bin1Path = "/Read/Tong1";
String bin1PickCount = "/Counter/fullrec";
String logIndexPath = "/Read/Tong1/index";
String hourPath = "/Read/Tong1/hour";
String minutePath = "/Read/Tong1/minute";
String pickupStatus;
String checkPickupStatus;
String status;
String date;
String day;
String dateFormat;

int hour;
int currentHour;
int nextHour;
int minute;
int currentMinute;
int nextMinute;
int currentSecond;
int daynumber;

// Relay
#define RELAY 26

// UltraSonic
#define TRIG_PIN 14
#define ECHO_PIN 27

// IR
#define IR_PIN 32

// Servo
Servo servo;
#define SERVO_PIN 33

// 7 Segment
MAX7219_7Seg_Disp disp(23,  5,  18);
int wdigit1, wdigit2, wfraction1, wfraction2, pdigit1, pdigit2;

// HX711
#define LOADCELL_DOUT_PIN 15
#define LOADCELL_SCK_PIN 22
HX711 scale;
float weight;
float kg;
float calib = 22.73;

// GPS
#define GPS_RX 16
#define GPS_TX 17
#define GPS_BAUD 9600
TinyGPSPlus gps;
double latitude;
double longitude;
// The serial connection to the GPS device
SoftwareSerial ss(GPS_RX, GPS_TX);

// UltraSonic settings
#define height 60
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

int jarak;
int percentage;
long duration;


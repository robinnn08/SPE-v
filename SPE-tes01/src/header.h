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
#define DATABASE_URL ""
#define API_KEY ""

#define FIREBASE_PROJECT_ID ""
#define FIREBASE_CLIENT_EMAIL ""
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\n\n-----END PRIVATE KEY-----\n";

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
float calib = 18.51;

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
#define height 100
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

int jarak;
int percentage;
long duration;


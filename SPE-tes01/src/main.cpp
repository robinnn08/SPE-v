#include "header.h"

void wifiSetup(){
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.mode(WIFI_STA);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void Open_Bin() {
  if (digitalRead(IR_PIN) == HIGH) {
    servo.write(180);
  }
  else {
    servo.write(0);
  }
}

void getLocation() {
  if (ss.available() > 0) {
    Serial.println("GPS Serial Connection Available");
    gps.encode(ss.read());
    if (gps.location.isValid()) {
      latitude = (gps.location.lat());
      longitude = (gps.location.lng());
    }
    else {
      latitude = 0.0;
      longitude = 0.0;
    }
  } 
  else {
    Serial.println("GPS Serial Connection Not Available");
  }
}

float roundToDecimalPlaces(float value, int decimalPlaces) {
    float multiplier = std::pow(10.0, decimalPlaces);
    return std::round(value * multiplier) / multiplier;
}


void serialPrint(){
  // Get Percentage
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH);
  jarak = duration * SOUND_SPEED / 2;

  percentage = (height - jarak) * 100 / height;

  // Get Weight
  if (!scale.is_ready()) {
    Serial.println("HX711 not found.");
  }
  else {
    weight = scale.get_units();
    if (weight < 0.0) {
      weight = 0.0; // Set negative readings to 0
    }
    kg = roundToDecimalPlaces((weight/1000), 2);
  }

  if (percentage >= 95) {
    pickupStatus = "Ready for pickup";
    status = "Available";
  }
  else {
    pickupStatus = "Not ready yet";
    status = "Full";
  }
  getLocation();

  Serial.print("Weight: ");
  Serial.print(weight);
  Serial.println(" g");

  Serial.print("Kilos: ");
  Serial.print(kg);
  Serial.println(" kg");

  Serial.print("Percentage: ");
  Serial.print(percentage);
  Serial.println(" %");

  Serial.print("Distance: ");
  Serial.print(jarak);
  Serial.println(" cm");

  Serial.print("Latitude: ");
  Serial.println(latitude, 6);
  Serial.print("Longitude: ");
  Serial.println(longitude, 6);

  Serial.println();
}

void Seven_Segment() {
  if (!isnan(weight) && !isnan(percentage)) {
    disp.Initialize(10);
    disp.Clear();
    wdigit1 = int(kg) / 10;
    wdigit2 = int(kg) % 10;
    wfraction1 = int(kg * 10) % 10;
    wfraction2 = int(kg * 100) % 10;

    disp.Number(8, wdigit1);
    disp.Numberdp(7, wdigit2);
    disp.Number(6, wfraction1);
    disp.Number(5, wfraction2);

    pdigit1 = int(percentage) / 10;
    pdigit2 = int(percentage) % 10;

    disp.Number(3, pdigit1);
    disp.Number(2, pdigit2);
  }
  else {
    disp.Clear(); 
  }
}

void powerSwitching() {
  bool hasSwitched = false;
  
  while (!timeClient.update())
  {
    timeClient.forceUpdate();
  }

  int relayHour = timeClient.getHours();
  
  if (relayHour >= 18 && relayHour < 7 && hasSwitched == false) {
    digitalWrite(RELAY, HIGH);
    hasSwitched = true;
  }
  else if (relayHour >= 7 && relayHour < 18 && hasSwitched == true) {
    digitalWrite(RELAY, LOW);
    hasSwitched = false;
  }
}

void firebaseSetup(){
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the sevice account credentials and private key (required) */
  config.service_account.data.client_email = FIREBASE_CLIENT_EMAIL;
  config.service_account.data.project_id = FIREBASE_PROJECT_ID;
  config.service_account.data.private_key = PRIVATE_KEY;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  firebaseData.setBSSLBufferSize(12960 /* Rx buffer size in bytes from 512 - 16384 */, 12960 /* Tx buffer size in bytes from 512 - 16384 */);

  // Or use legacy authenticate method
  // config.database_url = DATABASE_URL;
  // config.signer.tokens.legacy_token = "<database secret>";

  // To connect without auth in Test Mode, see Authentications/TestMode/TestMode.ino

  Firebase.begin(&config, &auth);
}

void dataLogging (){
  bool hasUploaded = false; // indikator data log sudah dilakukan/belum
  while (!timeClient.update())
  {
    timeClient.forceUpdate();
  }

  String time = timeClient.getFormattedTime();
  currentHour = timeClient.getHours();
  currentMinute = timeClient.getMinutes();
  currentSecond = timeClient.getSeconds();
  daynumber = timeClient.getDay();
 
  dateFormat = timeClient.getFormattedDate();
  date = dateFormat.substring(0, 10);

  if (daynumber == 1) {
    day = "Mon";
  }
  else if (daynumber == 2) {
    day = "Tue";
  }
  else if (daynumber == 3) {
    day = "Wed";
  }
  else if (daynumber == 4) {
    day = "Thu";
  }
  else if (daynumber == 5) {
    day = "Fri";
  }
  else if (daynumber == 6) {
    day = "Sat";
  }
  else if (daynumber == 7) {
    day = "Sun";
  }
  
  Firebase.getInt(firebaseData, hourPath);
  hour = firebaseData.intData();
  Firebase.getInt(firebaseData, minutePath);
  minute = firebaseData.intData();

  Serial.print("Data Logging: ");
  Serial.println(hasUploaded);
  Serial.println("Scheduled time: ");
  Serial.print(hour);
  Serial.println(minute);
  Serial.println("Current time: ");
  Serial.print(currentHour);
  Serial.println(currentMinute);

  nextMinute = minute + 1 % 60;
  if (nextMinute == 0) {
    nextHour = hour + 1 % 24;
  }
  else {
    nextHour = hour;
  }

  if (currentHour == hour && currentMinute == minute && currentSecond >= 0 && currentSecond <= 59 && hasUploaded == false)
    {
      Serial.print("we uploading data log\n");
      Firebase.getInt(firebaseData, logIndexPath);
      int logIndex = firebaseData.intData();
      int nextIndex = logIndex + 1;
      std::string indexPath = "/LogTest/" + std::to_string(logIndex);
    
      json.clear();
      json.set("/fullness", percentage);
      json.set("/weight", kg);
      json.set("/status", status);
      json.set("/date", date);
      json.set("/day", day);
      json.set("/time", time);

      Firebase.setJSON(firebaseData, indexPath, json);
      Firebase.setInt(firebaseData, logIndexPath, nextIndex);
      hasUploaded = true; // Set the flag to indicate that upload has been done
    }
    else if (currentHour == nextHour && currentMinute == nextMinute && currentSecond >= 0 && currentSecond <= 59 && hasUploaded == true) {
      Serial.print("ayy we done logging\n");
      hasUploaded = false; // Reset the flag to allow upload in the next interval
    }

}

void sendFirebase() {
  Serial.print("Firebase connection status: ");
  Serial.println(Firebase.ready() ? "true" : "false");
  // Check if Firebase connection is ready
  if (Firebase.ready()) {
    if (percentage >= 95) {
      pickupStatus = "Ready for pickup";
    } 
    else {
      pickupStatus = "Not ready yet";
    }

    json.clear();
    json.set("/capacity1", percentage);
    json.set("/weight1", kg);
    json.set("/status", status);
    json.set("/pick", pickupStatus);
    json.set("/lat", latitude);
    json.set("/long", longitude);
    Firebase.updateNode(firebaseData, bin1Path, json);

    if (pickupStatus != checkPickupStatus) {
      checkPickupStatus = pickupStatus;
      
      if (checkPickupStatus == "Ready for pickup") {
        Firebase.getInt(firebaseData, bin1PickCount);
        int counter = firebaseData.intData();
        int counterPlus = counter + 1;
        Firebase.setInt(firebaseData, bin1PickCount, counterPlus);
      } 
      else if (checkPickupStatus == "Not ready yet") { 
        Firebase.getInt(firebaseData, bin1PickCount);
        int counter = firebaseData.intData();
        int counterMin = counter - 1;
        int updatedCounter = max(counterMin, 0);
        Firebase.setInt(firebaseData, bin1PickCount, updatedCounter);
      }
    }
    dataLogging();
  } 
  else {
    Serial.println("Firebase not ready");
    firebaseSetup();
  }
}

void taskDataSerial(void* parameter) {
  while (1) {
    if (xSemaphoreTake(cSemaphore, portMAX_DELAY)) {
      serialPrint();
      xSemaphoreGive(cSemaphore);
    }
    else {
      Serial.println("Failed to take semaphore");
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void taskSegment(void* parameter) {
  while (1) {
    if (xSemaphoreTake(cSemaphore, portMAX_DELAY)) {
      Seven_Segment();
      xSemaphoreGive(cSemaphore);
    }
    else {
      Serial.println("Failed to take semaphore");
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void taskPowerSwitch(void* parameter) {
  while (1) {
    if (xSemaphoreTake(cSemaphore, portMAX_DELAY)) {
      powerSwitching();
      xSemaphoreGive(cSemaphore);
    }
    else {
      Serial.println("Failed to take semaphore");
    }
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}

void taskFirebase(void* parameter) {
  while (1) {
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
      sendFirebase();
      xSemaphoreGive(mutex);
    }
    else {
      Serial.println("Failed to take mutex");
    }
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}

void setup() {
  Serial.begin(115200);

  // Relay
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, LOW);

  // UltraSonic
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // IR
  pinMode(IR_PIN, INPUT);

  // Servo
  servo.attach(SERVO_PIN);
  servo.write(0); // Close the bin

  // HX711
  Serial.println("Initializing Scale....");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(calib);
  scale.tare();
  Serial.println("Scale Initialized");
  // 7 Segment
  Serial.println("Initializing 7 Segment....");
  disp.Initialize(10);
  disp.Clear();
  Serial.println("7 Segment Initialized");

  // GPS
  ss.begin(GPS_BAUD);

  wifiSetup();
  
  // NTP
  timeClient.begin();
  timeClient.update();
  timeClient.setTimeOffset(25200);

  firebaseSetup();
  delay(20000); // Untuk nunggu token dari firebase

  xTaskCreate(taskFirebase, "Firebase", configMINIMAL_STACK_SIZE + 20480, NULL, 0, NULL);
  xTaskCreate(taskDataSerial, "Serial", configMINIMAL_STACK_SIZE + 4096, NULL, 0, NULL);
  xTaskCreate(taskPowerSwitch, "Power", configMINIMAL_STACK_SIZE + 2048, NULL, 0, NULL);
  xTaskCreate(taskSegment, "Segment", configMINIMAL_STACK_SIZE + 2048, NULL, 0, NULL);
  // xTaskCreate(taskServo, "Servo", configMINIMAL_STACK_SIZE + 2048, NULL, 0, NULL);
}

void loop() {
  Open_Bin();
  
}

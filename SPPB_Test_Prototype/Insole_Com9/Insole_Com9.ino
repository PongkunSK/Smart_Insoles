#include <esp_now.h>
#include <WiFi.h>
#include <Arduino.h>

#define RED_PIN 5
#define GREEN_PIN 6
#define BLUE_PIN 7

const int maxDataPoints = 200;  // Adjust based on your needs
// int collectedData[maxDataPoints][4]; // Stores LFsensor, LBsensor, RFsensor, RBsensor
// int dataPointCount = 0;

unsigned long ledOnTime = 0;  // Stores the time when LED was turned on
bool ledOn = false;

// MAC address of Device 2
uint8_t device2Address[] = { 0x80, 0x65, 0x99, 0x6A, 0xD4, 0xBC };  // Replace with Device 2 MAC

// IR Command and Test States
uint32_t receivedIRCommand = 0;  // Store the IR command received from Device 2
enum TestState { IDLE,
                 TIME_UP_GO,
                 BALANCE1,
                 BALANCE2,
                 BALANCE3,
                 CHAIR_STAND_UP };
TestState currentTest = IDLE;

// Data structure to receive and send FSR values
typedef struct {
  int LFsensor;  // Value from A0 on Device 1
  int LBsensor;  // Value from A1 on Device 1
  int RFsensor;  // Value from A0 on Device 2
  int RBsensor;  // Value from A1 on Device 2
  char TUNGTresult;
  char SBSTresult;
  char STSTresult;
  char TDSTresult;
  char CSUTresult;
} FSRData;
FSRData fsrData;  // Instance to store received and send FSR data
//test result

char score;

// Time Up & Go Test Variables
unsigned long walkStartTime = 0;
unsigned long walkDuration = 0;
unsigned long lastWalkTime = 0;
const int LFWalkThreshold = 4095;
// const int LFWalkThreshold = 300;
const int RFWalkThreshold = 300;
const int LBWalkThreshold = 4095;
// const int LBWalkThreshold = 2800;
const int RBWalkThreshold = 3300;
const unsigned long inactivityLimit = 0;
bool isWalking = false;

// Balance Test Variables
const int SBSbalanceThreshold = 4095;
// const int SBSbalanceThreshold = 1500;
const int STSbalanceThreshold = 4095;
// const int STSbalanceThreshold = 1500;
const int TDSbalanceThreshold = 4095; 
// const int TDSbalanceThreshold = 1500; 
const int SBSsensorErrorMargin = 500;
const int STSsensorErrorMargin = 500;
const int TDSsensorErrorMargin = 500;
const unsigned long SBSbalanceTimeLimit = 10000;
const unsigned long STSbalanceTimeLimit = 10000;
const unsigned long TDSbalanceTimeLimit = 10000;
unsigned long SBSbalanceStartTime = 0;
unsigned long STSbalanceStartTime = 0;
unsigned long TDSbalanceStartTime = 0;
bool SBSisBalancing = false;
bool STSisBalancing = false;
bool TDSisBalancing = false;


// Chair Stand Up Test Variables
const int RFSitThreshold = 500;
const int LFSitThreshold = 4095;
// const int LFSitThreshold = 200;
const int RBSitThreshold = 2700;
const int LBSitThreshold = 4095;
// const int LBSitThreshold = 3200;
unsigned long startTime = 0;
unsigned long endTime = 0;
unsigned long totalTime = 0;
int repetitionCount = 0;
bool isSitting = true;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sent successfully" : "Send failed");
}

void OnDataRecv(const esp_now_recv_info *recv_info, const uint8_t *data, int len) {
  if (len == sizeof(FSRData)) {
    // Copy incoming data to the FSRData structure
    memcpy(&fsrData, data, len);
  } else if (len == sizeof(receivedIRCommand)) {
    memcpy(&receivedIRCommand, data, len);
    Serial.print("IR Command Received: ");
    Serial.println(receivedIRCommand, HEX);
    switch (receivedIRCommand) {  // add ปุ่มได้
      case 0x45:                  // Example: Time Up & Go Test
        currentTest = TIME_UP_GO;
        Serial.println("Switching to Time Up & Go Test.");
        break;
      case 0x44:  // side by side Test
        currentTest = BALANCE1;
        break;
      case 0x40:  // semi tandem Test
        currentTest = BALANCE2;
        break;
      case 0x43:  // tandem Test
        currentTest = BALANCE3;
        break;
      case 0x7:  // Chair Stand Test
        currentTest = CHAIR_STAND_UP;
        break;
      case 0x1c:
        Serial.println("Sending all test results to the server...");
        // sendTestResults(String(fsrData.TUNGTresult), String(fsrData.SBSresult), String(fsrData.STSresult), String(fsrData.TDSresult), String(fsrData.CSUTresult));
        break;
      default:
        Serial.println("Unknown command received.");
        break;
    }
  }
}

void setRGBColor(int red, int green, int blue) {
  analogWrite(RED_PIN, 255 - red);  // Invert values for common anode
  analogWrite(GREEN_PIN, 255 - green);
  analogWrite(BLUE_PIN, 255 - blue);
}
void PassOrFailTUNGT() {
  if (fsrData.TUNGTresult == 'P') {
    setRGBColor(0, 255, 0);  // Turn on RGB LED to Magenta (Red + Blue)
    ledOnTime = millis();    // Record the time when the LED was turned on
    ledOn = true;
    if (ledOn && millis() - ledOnTime >= 1000) {
      setRGBColor(0, 0, 0);  // Turn off the RGB LED
      ledOn = false;         // Reset the flag to indicate LED is off
    }
  } else if (fsrData.TUNGTresult == 'F') {
    setRGBColor(255, 0, 0);  // Turn on RGB LED to Magenta (Red + Blue)
    ledOnTime = millis();    // Record the time when the LED was turned on
    ledOn = true;
    if (ledOn && millis() - ledOnTime >= 1000) {
      setRGBColor(0, 0, 0);  // Turn off the RGB LED
      ledOn = false;         // Reset the flag to indicate LED is off
    }
  }
}

void PassOrFailSBS() {
  if (fsrData.SBSTresult == 'P') {
    setRGBColor(0, 255, 0);  // Turn on RGB LED to Magenta (Red + Blue)
    ledOnTime = millis();    // Record the time when the LED was turned on
    ledOn = true;
    if (ledOn && millis() - ledOnTime >= 1000) {
      setRGBColor(0, 0, 0);  // Turn off the RGB LED
      ledOn = false;         // Reset the flag to indicate LED is off
    }
  } else if (fsrData.SBSTresult == 'F') {
    setRGBColor(255, 0, 0);  // Turn on RGB LED to Magenta (Red + Blue)
    ledOnTime = millis();    // Record the time when the LED was turned on
    ledOn = true;
    if (ledOn && millis() - ledOnTime >= 1000) {
      setRGBColor(0, 0, 0);  // Turn off the RGB LED
      ledOn = false;         // Reset the flag to indicate LED is off
    }
  }
}

void PassOrFailSTS() {
  if (fsrData.STSTresult == 'P') {
    setRGBColor(0, 255, 0);  // Turn on RGB LED to Magenta (Red + Blue)
    ledOnTime = millis();    // Record the time when the LED was turned on
    ledOn = true;
    if (ledOn && millis() - ledOnTime >= 1000) {
      setRGBColor(0, 0, 0);  // Turn off the RGB LED
      ledOn = false;         // Reset the flag to indicate LED is off
    }
  } else if (fsrData.STSTresult == 'F') {
    setRGBColor(255, 0, 0);  // Turn on RGB LED to Red
    ledOnTime = millis();    // Record the time when the LED was turned on
    ledOn = true;
    if (ledOn && millis() - ledOnTime >= 1000) {
      setRGBColor(0, 0, 0);  // Turn off the RGB LED
      ledOn = false;         // Reset the flag to indicate LED is off
    }
  }
}

void PassOrFailTDS() {
  if (fsrData.TDSTresult == 'P') {
    setRGBColor(0, 255, 0);  // Turn on RGB LED to Green
    ledOnTime = millis();    // Record the time when the LED was turned on
    ledOn = true;
    if (ledOn && millis() - ledOnTime >= 1000) {
      setRGBColor(0, 0, 0);  // Turn off the RGB LED
      ledOn = false;         // Reset the flag to indicate LED is off
    }
  } else if (fsrData.TDSTresult == 'F') {
    setRGBColor(255, 0, 0);  // Turn on RGB LED to Magenta (Red + Blue)
    ledOnTime = millis();    // Record the time when the LED was turned on
    ledOn = true;
    if (ledOn && millis() - ledOnTime >= 1000) {
      setRGBColor(0, 0, 0);  // Turn off the RGB LED
      ledOn = false;         // Reset the flag to indicate LED is off
    }
  }
}

void timeUpAndGoTest() {
  if (!ledOn) {
    setRGBColor(255, 0, 255);  // Turn on RGB LED to Magenta (Red + Blue)
    ledOnTime = millis();      // Record the time when the LED was turned on
    ledOn = true;              // Set the flag indicating LED is on
  }

  // Check if 1 second has passed since turning on the LED
  if (ledOn && millis() - ledOnTime >= 1000) {
    setRGBColor(0, 0, 0);  // Turn off the RGB LED
    ledOn = false;         // Reset the flag to indicate LED is off
  }
  Serial.println("----Time Up & Go Test Start----");
  Serial.print("RightFrontsensor: ");
  Serial.print(fsrData.RFsensor);
  Serial.print(", LeftFrontsensor: ");
  Serial.print(fsrData.LFsensor);
  Serial.print(", RightBacksensor: ");
  Serial.print(fsrData.RBsensor);
  Serial.print(", Left Back Sensor Value: ");
  Serial.println(fsrData.LBsensor);
  // if ((fsrData.RFsensor >= RFWalkThreshold && fsrData.LFsensor >= LFWalkThreshold) || (fsrData.RBsensor >= RBWalkThreshold && fsrData.LBsensor >= LBWalkThreshold)) {
  if ((fsrData.RFsensor >= RFWalkThreshold && fsrData.LFsensor >= LFWalkThreshold) || (fsrData.RBsensor >= RBWalkThreshold && fsrData.LBsensor >= LBWalkThreshold)) {
    if (!isWalking) {
      isWalking = true;
      walkStartTime = millis();
      Serial.println("Walking Start...");
    }
  } else if (isWalking) {
    // if (fsrData.RFsensor <= 200 && fsrData.LFsensor <= 200 && fsrData.RBsensor <= 2700 && fsrData.LBsensor <= 3200) {
    if (fsrData.RFsensor <= 200 && fsrData.LFsensor < 4095 && fsrData.RBsensor <= 2500 && fsrData.LBsensor < 4095) {
      walkDuration = millis() - walkStartTime;
      Serial.print("Walking duration: ");
      Serial.print(walkDuration / 1000.0);
      Serial.println(" seconds.");

      // Grading
      fsrData.TUNGTresult = (walkDuration <= 13500) ? 'P' : 'F';
      Serial.print("Test Result: ");
      Serial.println(fsrData.TUNGTresult == 'P' ? "Pass" : "Fail");
      PassOrFailTUNGT();
      currentTest = IDLE;
      isWalking = false;
    }
  }
  // if (fsrData.RFsensor <= 200 && fsrData.LFsensor <= 200 && fsrData.RBsensor <= 500 && fsrData.LBsensor <= 200) {
  if (fsrData.RFsensor <= 200 && fsrData.LFsensor < 4095 && fsrData.RBsensor <= 500 && fsrData.LBsensor < 4095) {
    Serial.println("Immediate stop: All sensors ≤ 200. Test failed.");
    fsrData.TUNGTresult = 'F';
    PassOrFailTUNGT();
    currentTest = IDLE;
    isWalking = false;
    return;
  }
}

// Side by Side
void balanceTestSBS() {
  if (!ledOn) {
    setRGBColor(0, 0, 255);  // Turn on RGB LED to Blue
    ledOnTime = millis();    // Record the time when the LED was turned on
    ledOn = true;            // Set the flag indicating LED is on
  }
  // Check if 1 second has passed since turning on the LED
  if (ledOn && millis() - ledOnTime >= 1000) {
    setRGBColor(0, 0, 0);  // Turn off the RGB LED
    ledOn = false;         // Reset the flag to indicate LED is off
  }
  int adjustedLF = fsrData.LFsensor + SBSsensorErrorMargin;
  int adjustedRF = fsrData.RFsensor + SBSsensorErrorMargin;
  int adjustedLB = fsrData.LBsensor + SBSsensorErrorMargin;
  int adjustedRB = fsrData.RBsensor + SBSsensorErrorMargin;

  Serial.println("----Balance Test Start----");
  Serial.print("RightFrontsensor: ");
  Serial.print(fsrData.RFsensor);
  Serial.print(", LeftFrontsensor: ");
  Serial.print(fsrData.LFsensor);
  Serial.print(", RightBacksensor: ");
  Serial.print(fsrData.RBsensor);
  Serial.print(", Left Back Sensor Value: ");
  Serial.println(fsrData.LBsensor);

  // Check if all sensor values are below 500
  // if (fsrData.LFsensor < 500 && fsrData.RFsensor < 500 && fsrData.LBsensor < 500 && fsrData.RBsensor < 500) {
  if (fsrData.LFsensor < 4095 && fsrData.RFsensor < 500 && fsrData.LBsensor < 4095 && fsrData.RBsensor < 500) {
    Serial.println("All sensor values are below 500. Test Failed.");
    fsrData.SBSTresult = 'F';
    PassOrFailSBS();

    currentTest = IDLE;  // Stop the test
    return;
  }

  if (abs(adjustedLF - adjustedRF) <= SBSbalanceThreshold && abs(adjustedLB - adjustedRB) <= SBSbalanceThreshold) {
    if (!SBSisBalancing) {
      SBSisBalancing = true;
      SBSbalanceStartTime = millis();
      Serial.println("Balancing...");
    }
    if (millis() - SBSbalanceStartTime >= SBSbalanceTimeLimit) {

      Serial.println("Balance Test Passed!");
      // Set result as Pass
      fsrData.SBSTresult = 'P';
      PassOrFailSBS();
      Serial.println("Result: Pass");
      // Mark test as completed
      currentTest = IDLE;  // Stop the test
    }
  }

  // If balance is not achieved in time
  if (!SBSisBalancing && (millis() - SBSbalanceStartTime <= SBSbalanceTimeLimit)) {

    fsrData.SBSTresult = 'F';
    PassOrFailSBS();
    Serial.println("Result: Fail");
    // Mark test as completed
    currentTest = IDLE;  // Stop the test
  }
}
// Semi Tandem
void balanceTestSTS() {
  if (!ledOn) {
    setRGBColor(0, 128, 255);  // Turn on RGB LED to Blue
    ledOnTime = millis();    // Record the time when the LED was turned on
    ledOn = true;            // Set the flag indicating LED is on
  }
  // Check if 1 second has passed since turning on the LED
  if (ledOn && millis() - ledOnTime >= 1000) {
    setRGBColor(0, 0, 0);  // Turn off the RGB LED
    ledOn = false;         // Reset the flag to indicate LED is off
  }
  int adjustedLF = fsrData.LFsensor + STSsensorErrorMargin;
  int adjustedRF = fsrData.RFsensor + STSsensorErrorMargin;
  int adjustedLB = fsrData.LBsensor + STSsensorErrorMargin;
  int adjustedRB = fsrData.RBsensor + STSsensorErrorMargin;

  Serial.println("----Balance Test Start----");
  Serial.print("RightFrontsensor: ");
  Serial.print(fsrData.RFsensor);
  Serial.print(", LeftFrontsensor: ");
  Serial.print(fsrData.LFsensor);
  Serial.print(", RightBacksensor: ");
  Serial.print(fsrData.RBsensor);
  Serial.print(", Left Back Sensor Value: ");
  Serial.println(fsrData.LBsensor);

  // Check if all sensor values are below 500
  // if (fsrData.LFsensor < 500 && fsrData.RFsensor < 500 && fsrData.LBsensor < 500 && fsrData.RBsensor < 500) {
  if (fsrData.LFsensor < 4095 && fsrData.RFsensor < 500 && fsrData.LBsensor < 4095 && fsrData.RBsensor < 500) {
    Serial.println("All sensor values are below 500. Test Failed.");
    fsrData.STSTresult = 'F';
    PassOrFailSTS();

    currentTest = IDLE;  // Stop the test
    return;
  }

  if (abs(adjustedLF - adjustedRF) <= STSbalanceThreshold && abs(adjustedLB - adjustedRB) <= STSbalanceThreshold) {
  // if (abs(adjustedLF - adjustedRF) <= STSbalanceThreshold && abs(adjustedLB - adjustedRB) <= STSbalanceThreshold) {
    if (!STSisBalancing) {
      STSisBalancing = true;
      STSbalanceStartTime = millis();
      Serial.println("Balancing...");
    }
    if (millis() - STSbalanceStartTime >= STSbalanceTimeLimit) {

      Serial.println("Balance Test Passed!");
      // Set result as Pass
      fsrData.STSTresult = 'P';
      PassOrFailSTS();
      Serial.println("Result: Pass");
      // Mark test as completed
      currentTest = IDLE;  // Stop the test
    }
  }

  // If balance is not achieved in time
  if (!STSisBalancing && (millis() - STSbalanceStartTime <= STSbalanceTimeLimit)) {

    fsrData.STSTresult = 'F';
    PassOrFailSTS();
    Serial.println("Result: Fail");
    // Mark test as completed
    currentTest = IDLE;  // Stop the test
  }
}

// Tandem
void balanceTestTDS() {
  if (!ledOn) {
    setRGBColor(0, 255, 128);  // Turn on RGB LED to Blue
    ledOnTime = millis();    // Record the time when the LED was turned on
    ledOn = true;            // Set the flag indicating LED is on
  }
  // Check if 1 second has passed since turning on the LED
  if (ledOn && millis() - ledOnTime >= 1000) {
    setRGBColor(0, 0, 0);  // Turn off the RGB LED
    ledOn = false;         // Reset the flag to indicate LED is off
  }
  int adjustedLF = fsrData.LFsensor + TDSsensorErrorMargin;
  int adjustedRF = fsrData.RFsensor + TDSsensorErrorMargin;
  int adjustedLB = fsrData.LBsensor + TDSsensorErrorMargin;
  int adjustedRB = fsrData.RBsensor + TDSsensorErrorMargin;

  Serial.println("----Balance Test Start----");
  Serial.print("RightFrontsensor: ");
  Serial.print(fsrData.RFsensor);
  Serial.print(", LeftFrontsensor: ");
  Serial.print(fsrData.LFsensor);
  Serial.print(", RightBacksensor: ");
  Serial.print(fsrData.RBsensor);
  Serial.print(", Left Back Sensor Value: ");
  Serial.println(fsrData.LBsensor);

  // Check if all sensor values are below 500
  // if (fsrData.LFsensor < 500 && fsrData.RFsensor < 500 && fsrData.LBsensor < 500 && fsrData.RBsensor < 500) {
  if (fsrData.LFsensor < 4095 && fsrData.RFsensor < 500 && fsrData.LBsensor < 4095 && fsrData.RBsensor < 500) {
    Serial.println("All sensor values are below 500. Test Failed.");
    fsrData.TDSTresult = 'F';
    PassOrFailTDS();

    currentTest = IDLE;  // Stop the test
    return;
  }

  // if (abs(adjustedLF - adjustedRF) <= SBSbalanceThreshold && abs(adjustedLB - adjustedRB) <= SBSbalanceThreshold) {
  if (abs(adjustedLF - adjustedRF) <= TDSbalanceThreshold && abs(adjustedLB - adjustedRB) <= TDSbalanceThreshold) {
    if (!TDSisBalancing) {
      TDSisBalancing = true;
      TDSbalanceStartTime = millis();
      Serial.println("Balancing...");
    }
    if (millis() - TDSbalanceStartTime >= TDSbalanceTimeLimit) {

      Serial.println("Balance Test Passed!");
      // Set result as Pass
      fsrData.TDSTresult = 'P';
      PassOrFailTDS();
      Serial.println("Result: Pass");
      // Mark test as completed
      currentTest = IDLE;  // Stop the test
    }
  }

  // If balance is not achieved in time
  if (!TDSisBalancing && (millis() - TDSbalanceStartTime <= TDSbalanceTimeLimit)) {

    fsrData.TDSTresult = 'F';
    PassOrFailTDS();
    Serial.println("Result: Fail");
    // Mark test as completed
    currentTest = IDLE;  // Stop the test
  }
}

void PassOrFailCSUT() {
  if (fsrData.CSUTresult == '4' || fsrData.CSUTresult == '3' || fsrData.CSUTresult == '2' || fsrData.CSUTresult == '1') {
    setRGBColor(0, 255, 0);  // Turn on RGB LED to Green
    ledOnTime = millis();    // Record the time when the LED was turned on
    ledOn = true;
    if (ledOn && millis() - ledOnTime >= 1000) {
      setRGBColor(0, 0, 0);  // Turn off the RGB LED
      ledOn = false;         // Reset the flag to indicate LED is off
    }
  } else if (score == '0') {
    setRGBColor(255, 0, 0);  // Turn on RGB LED to Magenta (Red + Blue)
    ledOnTime = millis();    // Record the time when the LED was turned on
    ledOn = true;
    if (ledOn && millis() - ledOnTime >= 1000) {
      setRGBColor(0, 0, 0);  // Turn off the RGB LED
      ledOn = false;         // Reset the flag to indicate LED is off
    }
  }
}
// Chair Stand Up Test
void chairStandUpTest() {
  if (!ledOn) {
    setRGBColor(255, 128, 0);  // Turn on RGB LED to Blue
    ledOnTime = millis();
    ledOn = true;
  }

  if (ledOn && millis() - ledOnTime >= 1000) {
    setRGBColor(0, 0, 0);  // Turn off the RGB LED
    ledOn = false;
  }
  Serial.println("----Chair Stand Up Test Start----");
  Serial.println("Waiting for sit down...");
  Serial.print("RightFrontsensor: ");
  Serial.print(fsrData.RFsensor);
  Serial.print(", LeftFrontsensor: ");
  Serial.print(fsrData.LFsensor);
  Serial.print(", RightBacksensor: ");
  Serial.print(fsrData.RBsensor);
  Serial.print(", Left Back Sensor Value: ");
  Serial.println(fsrData.LBsensor);

  if (fsrData.LFsensor <= 4095 && fsrData.RFsensor < 200 && fsrData.LBsensor <= 4095 && fsrData.RBsensor < 300) {
  // if (fsrData.LFsensor <= 200 && fsrData.RFsensor < 200 && fsrData.LBsensor <= 200 && fsrData.RBsensor < 200) {
    Serial.println("All sensor values are below 500. Test Failed.");
    score = '0';
    if (!ledOn) {
      setRGBColor(255, 0, 0);  // Turn on RGB LED to Magenta (Red + Blue)
      ledOnTime = millis();    // Record the time when the LED was turned on
      ledOn = true;            // Set the flag indicating LED is on
    }
    // Check if 1 second has passed since turning on the LED
    if (ledOn && millis() - ledOnTime >= 1000) {
      setRGBColor(0, 0, 0);  // Turn off the RGB LED
      ledOn = false;         // Reset the flag to indicate LED is off
    }

    currentTest = IDLE;  // Stop the test
    return;
  }

  // bool currentlySitting = (fsrData.RFsensor < RFSitThreshold && fsrData.LFsensor < LFSitThreshold && fsrData.RBsensor < RBSitThreshold && fsrData.LBsensor < LBSitThreshold);
  bool currentlySitting = (fsrData.RBsensor <= RBSitThreshold && fsrData.LBsensor <= LBSitThreshold);

  if (currentlySitting && !isSitting) {
    endTime = millis();
    totalTime += (endTime - startTime);
    repetitionCount++;
    Serial.print("Repetition ");
    Serial.print(repetitionCount);
    Serial.print(" completed in ");
    Serial.print((endTime - startTime) / 1000.0);
    Serial.println(" seconds.");

    if (repetitionCount == 5) {

      Serial.println("Test complete.");
      Serial.print("Total time: ");
      Serial.print(totalTime / 1000.0);
      Serial.println(" seconds.");

      // Scoring based on total time
      float totalTimeInSeconds = totalTime / 1000.0;
      fsrData.CSUTresult = '0';

      if (totalTimeInSeconds <= 11.19) {
        fsrData.CSUTresult = '4';

      } else if (totalTimeInSeconds <= 13.69) {
        fsrData.CSUTresult = '3';

      } else if (totalTimeInSeconds <= 16.69) {

        fsrData.CSUTresult = '2';

      } else if (totalTimeInSeconds <= 60.0) {

        fsrData.CSUTresult = '1';

      } else {
        fsrData.CSUTresult = '0';
      }

      PassOrFailCSUT();
      Serial.print("Score: ");
      Serial.println(fsrData.CSUTresult);
      // Set result as Pass
      // fsrData.CSUTresult = score;
      currentTest = IDLE;
    }
  }
  if (!currentlySitting && isSitting) {
    startTime = millis();
  }
  isSitting = currentlySitting;
}

void setup() {
  Serial.begin(115200);

  // Set RGB pins as OUTPUT and turn them off initially
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  WiFi.mode(WIFI_STA);  // Set Wi-Fi mode to Station

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register callback to receive data
  esp_now_register_recv_cb(OnDataRecv);

  // Add peer (Device 2)
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, device2Address, 6);
  peerInfo.channel = 0;      // Default channel
  peerInfo.encrypt = false;  // No encryption

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  Serial.println("Device 1 ready to receive commands.");
}

void loop() {
  setRGBColor(0, 0, 0);  // All OFF
  // Read FSR values from A0 and A1 (Left sensors)
  fsrData.LFsensor = analogRead(A2);  // Left Front sensor
  fsrData.LBsensor = analogRead(A3);  // Left Back sensor

  switch (currentTest) {
    case TIME_UP_GO:
      timeUpAndGoTest();
      if (currentTest == IDLE) {
        Serial.println("Time Up & Go Test Complete. Select another test (1, 2, or 3).");
      }
      break;
    case BALANCE1:
      balanceTestSBS();
      if (currentTest == IDLE) {
        Serial.println("Side By Side Test Complete. Select another test (1, 2, or 3).");
      }
      break;
    case BALANCE2:
      balanceTestSTS();
      if (currentTest == IDLE) {
        Serial.println("Semi Tandem Test Complete. Select another test (1, 2, or 3).");
      }
      break;
    case BALANCE3:
      balanceTestTDS();
      if (currentTest == IDLE) {
        Serial.println("Tandem Test Complete. Select another test (1, 2, or 3).");
      }
      break;

    case CHAIR_STAND_UP:
      chairStandUpTest();
      if (currentTest == IDLE) {
        Serial.println("Chair Stand Up Test Complete. Select another test (1, 2, or 3).");
      }
      break;
    default:
      Serial.println("Waiting for IR Command...");
      break;
  }

  // Send FSR data to Device 2 (make this more frequent)
  esp_now_send(device2Address, (uint8_t *)&fsrData, sizeof(fsrData));

  // Send data every 10ms for more frequent updates (reduce delay)
  delay(10);
}
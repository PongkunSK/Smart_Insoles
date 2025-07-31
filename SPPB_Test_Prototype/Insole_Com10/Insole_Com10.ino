///com10
#include <esp_now.h>
#include <WiFi.h>
#include <IRremote.hpp>
#define IR_RECEIVE_PIN 4
#include <HTTPClient.h>

const char *ssid = "SK_Home";
const char *password = "12022546";

const int maxDataPoints = 200;

// const char* ssid = "T0ngY1b";
// const char* password = "11111111";
// const char* ssid = "Tamjailoey 2.4";
// const char* password = "vairoonyadong";
const char *serverURL = "http://192.168.0.120:3000/api/test_results";
// const char* serverURL = "http://192.168.244.13:3000/api/test_results";
// const char* serverURL = "http://192.168.1.147:3000/api/test_results";

// Add a user ID to the device code
const String userId = "insole1";  // Or retrieve from memory or configuration

void sendTestResults(String tungt, String SBSresult, String STSresult, String TDSresult, String csut) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");
    // Include userId in the payload
    String payload = String("{\"userId\":\"") + userId + "\",\"TUNGTresult\":\"" + tungt + "\",\"SBSTresult\":\"" + SBSresult + "\",\"STSTresult\":\"" + STSresult + "\",\"TDSTresult\":\"" + TDSresult + "\",\"CSUTresult\":\"" + csut + "\"}";
    int httpResponseCode = http.POST(payload);

    Serial.println(httpResponseCode > 0 ? "Data sent!" : "Send failed!");
    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}

bool processPaused = false;

// MAC address of Device 1
uint8_t device1Address[] = { 0x80, 0x65, 0x99, 0x6A, 0xAD, 0x14 };  // Replace with Device 1 MAC
uint32_t irCommand = 0;

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

// Test States
enum TestState { IDLE,
                 TIME_UP_GO,
                 BALANCE,
                 CHAIR_STAND_UP };
TestState currentTest = IDLE;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sent successfully" : "Send failed");
}

void OnDataRecv(const esp_now_recv_info *recv_info, const uint8_t *data, int len) {
  if (len == sizeof(FSRData)) {
    // Copy incoming data to the FSRData structure
    memcpy(&fsrData, data, len);

    // Print the received TUNGTresult
    // Serial.print("Received TUNGTresult: ");
    // Serial.println(fsrData.TUNGTresult == 'P' ? "Pass" : "Fail");

  } else {
    Serial.println("Unexpected data format received.");
  }

  // Send FSR data from Device 2 (RFsensor and RBsensor) back to Device 1
  fsrData.RFsensor = analogRead(A0);  // Read RF sensor on Device 2
  fsrData.RBsensor = analogRead(A1);  // Read RB sensor on Device 2

  // Send the FSR data from Device 2 to Device 1
  esp_now_send(device1Address, (uint8_t *)&fsrData, sizeof(fsrData));
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  // Begin WiFi connection
  // WiFi.begin(ssid, password);
  // Serial.print("Connecting to WiFi...");

  // int retries = 0; // Limit retries to prevent infinite loops
  // while (WiFi.status() != WL_CONNECTED && retries < 20) {
  //   delay(500);
  //   Serial.print(".");
  //   retries++;
  // }

  // if (WiFi.status() == WL_CONNECTED) {
  //   Serial.println("\nWiFi connected successfully!");
  //   Serial.print("IP Address: ");
  //   Serial.println(WiFi.localIP());
  // } else {
  //   Serial.println("\nWiFi connection failed.");
  // }

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register callbacks
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);

  // Register peer (Device 1)
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, device1Address, 6);
  peerInfo.channel = 0;      // Default channel
  peerInfo.encrypt = false;  // No encryption

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);
  Serial.println("IR Receiver Ready. Select Test (1. Time Up & Go, 2. Balance, 3. Chair Stand Up)");
}

void loop() {
  // Process incoming IR data
  if (IrReceiver.decode()) {
    irCommand = IrReceiver.decodedIRData.command;
    switch (irCommand) {
      case 0x1c:
        WiFi.begin(ssid, password);
        Serial.print("Connecting to WiFi...");

        // int retries = 5;  // Limit retries to prevent infinite loops
        while (WiFi.status() != WL_CONNECTED) {
          Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("\nWiFi connected successfully!");
          Serial.print("IP Address: ");
          Serial.println(WiFi.localIP());
          Serial.println("Sending all test results to the server...");
          sendTestResults(String(fsrData.TUNGTresult), String(fsrData.SBSTresult), String(fsrData.STSTresult), String(fsrData.TDSTresult), String(fsrData.CSUTresult));
          Serial.println(fsrData.TUNGTresult);
          Serial.println(fsrData.SBSTresult);
          Serial.println(fsrData.STSTresult);
          Serial.println(fsrData.TDSTresult);
          Serial.println(fsrData.CSUTresult);
        } else {
          Serial.println("\nWiFi connection failed.");
        }

        break;
    }
    IrReceiver.resume();
    // Send the IR command to Device 1
    if (esp_now_send(device1Address, (uint8_t *)&irCommand, sizeof(irCommand)) != ESP_OK) {
      Serial.println("Error sending IR data.");
    } else {
      Serial.print("IR Command sent: ");
      Serial.println(irCommand, HEX);
    }
  }
  // Send FSR data
  esp_now_send(device1Address, (uint8_t *)&fsrData, sizeof(fsrData));
  delay(10);
}
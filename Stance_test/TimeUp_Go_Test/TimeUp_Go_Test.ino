#include <esp_now.h>

int RFSensor = 0;
//int RBSensor = 0;
unsigned long walkStartTime = 0;
unsigned long walkDuration = 0;
unsigned long lastWalkTime = 0; // To track the last time walking was detected
const int threshold = 50;
const unsigned long inactivityLimit = 5000; // 5 seconds in milliseconds
bool isWalking = false;

void setup()
{
  pinMode(A0, INPUT);
//  pinMode(A1, INPUT);
  Serial.begin(115200);
  Serial.println("Starting...");
}

void loop()
{
  RFSensor = analogRead(A0);
//  sensor2 = analogRead(A1);
  //int LFSensor = receivedData.sensorLeftFront;
  Serial.println("data...");
  Serial.print("Left Front Sensor: ");
  //Serial.println(LFSensor);
  Serial.print("Right Front Sensor: ");
  Serial.println(RFSensor);
  delay(100); // Wait for 100 milliseconds

  if (RFSensor >= threshold) {
    //if (LFSensor >= threshold || RFSensor >= threshold) {
    if (!isWalking) {
      // Walking started
      isWalking = true;
      walkStartTime = millis();
    }
    lastWalkTime = millis(); // Reset the inactivity timer
    Serial.println("Walking...");
  } else {
    if (isWalking) {
      // Walking stopped
      isWalking = false;
      walkDuration += millis() - walkStartTime;
      Serial.println("Not Walking");
    } else {
      Serial.println("Not Walking");
    }
  }

  // Check for inactivity over the threshold of 5 seconds
  if (!isWalking && (millis() - lastWalkTime >= inactivityLimit)) {
    Serial.println("No walking detected for more than 5 seconds. Stopping process...");
    Serial.print("Duration: ");
    Serial.print(walkDuration/1000);
    Serial.print("s");
    while (true) {
      // Infinite loop to stop the process
    }
  }

  // Display walk duration if walking is stopped
  if (!isWalking && walkDuration > 0) {
    Serial.print("Duration: ");
    Serial.print(walkDuration / 1000); // Convert milliseconds to seconds
    Serial.println("s");
  }

  Serial.println(); // Print a blank line for better readability
}

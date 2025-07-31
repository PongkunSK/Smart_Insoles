int sensorLeft = 0;
int sensorRight = 0;
const int balanceThreshold = 10; // Acceptable threshold for balance
const int sensorErrorMargin = 5; // Error margin for sensor inaccuracy
const unsigned long balanceTimeLimit = 10000; // 10 seconds in milliseconds
unsigned long balanceStartTime = 0;
bool isBalancing = false;
bool testComplete = false;

void setup()
{
  pinMode(A0, INPUT); // Left foot sensor
  pinMode(A1, INPUT); // Right foot sensor
  Serial.begin(115200);
  Serial.println("Balance Test Starting...");
}

void loop()
{
  // Read sensors
  sensorLeft = analogRead(A0);
  sensorRight = analogRead(A1);
  Serial.print("Left Sensor: ");
  Serial.println(sensorLeft);
  Serial.print("Right Sensor: ");
  Serial.println(sensorRight);
  delay(100); // Delay for readability

  // Check if the sensors are within the balance threshold, considering the error margin
  int adjustedLeft = sensorLeft + sensorErrorMargin;
  int adjustedRight = sensorRight + sensorErrorMargin;
  
  // Check if the difference between adjusted sensor values is within the balance threshold
  if (abs(adjustedLeft - adjustedRight) <= balanceThreshold && !testComplete) {
    if (!isBalancing) {
      // Start balance timer
      isBalancing = true;
      balanceStartTime = millis();
      Serial.println("Balancing...");
    }
    
    // Check if 10 seconds of stability have passed
    if (millis() - balanceStartTime >= balanceTimeLimit) {
      Serial.println("Balance Test Passed: Maintained balance for 10 seconds!");
      testComplete = true; // End the test
    }
  } else {
    if (isBalancing) {
      // Reset balance timer if balance is lost
      isBalancing = false;
      Serial.println("Unstable! Balance test reset.");
    }
  }

  // Display progress if still balancing but not complete
  if (isBalancing && !testComplete) {
    Serial.print("Balancing for ");
    Serial.print((millis() - balanceStartTime) / 1000);
    Serial.println(" seconds...");
  }
  
  // Optional: Stop further readings once test is complete
  if (testComplete) {
    while (true) {
      // Infinite loop to stop the program
    }
  }

  Serial.println(); // Blank line for readability
}

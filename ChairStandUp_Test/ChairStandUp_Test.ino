// Define the pin numbers for the FSR sensors
const int FSR1 = A0; // Left shoe - sensor 1
const int FSR2 = A1; // Left shoe - sensor 2
//const int FSR3 = A2; // Right shoe - sensor 1
//const int FSR4 = A3; // Right shoe - sensor 2

// Define the thresholds for detecting standing vs. sitting
const int sitThreshold = 500; // Sensor value for "sitting"
const int standThreshold = 501; // Sensor value for "standing"

// Variables to hold time durations
unsigned long startTime = 0;
unsigned long endTime = 0;
unsigned long totalTime = 0;
int repetitionCount = 0; // To track how many times the user has stood up and sat down

// Define state variables
bool isSitting = true; // Assuming the test starts with the person sitting

void setup() {
  // Initialize the serial communication
  Serial.begin(115200);
  
  // Set FSR pins as input
  pinMode(FSR1, INPUT);
  pinMode(FSR2, INPUT);
  //pinMode(FSR3, INPUT);
  //pinMode(FSR4, INPUT);
  
  // Print initial message
  //Serial.println("Chair Stand Test started. Please follow the instructions.");
  //Serial.println("You will need to stand up and sit down 5 times.");
  Serial.println("Waiting for sit down...");
}

void loop() {
  // Read sensor values
  int sensor1 = analogRead(FSR1);
  int sensor2 = analogRead(FSR2);
  //int sensor3 = analogRead(FSR3);
  //int sensor4 = analogRead(FSR4);

  // Check if the person is sitting or standing
  bool currentlySitting = (sensor1 < sitThreshold && sensor2 < sitThreshold);
  //bool currentlySitting = (sensor1 < sitThreshold && sensor2 < sitThreshold && sensor3 < sitThreshold && sensor4 < sitThreshold);
  if (currentlySitting && !isSitting) {
    // Transition from standing to sitting (end of standing event)
    endTime = millis();
    totalTime += (endTime - startTime);
    Serial.print("Time for stand-up and sit-down #");
    Serial.print(repetitionCount + 1);
    Serial.print(": ");
    Serial.print((endTime - startTime) / 1000.0);
    Serial.println(" seconds.");
    Serial.println(sensor1);
    Serial.println(sensor2);
    // Increment the repetition count
    repetitionCount++;

    // Check if the test is complete (5 repetitions)
    if (repetitionCount >= 5) {
      Serial.println("Test complete. Total time: ");
      Serial.print(totalTime / 1000.0); // Display total time in seconds
      Serial.println(" seconds.");
      Serial.println("Test finished.");
      while (true); // Stop the test
    }
  }

  if (!currentlySitting && isSitting) {
    // Transition from sitting to standing (start of standing event)
    startTime = millis();
  }

  // Update the sitting/standing state
  isSitting = currentlySitting;
}

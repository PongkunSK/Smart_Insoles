# Smart Insoles

This repository is for the **Smart Insoles** senior project.  
It contains a **web application** to observe data from the device through a **MongoDB database**.

The main purpose is to develop an **automatic health care data collection system** with insoles using pressure sensors, especially for the elderly who are at risk of falling.  
The insole devices are based on **low-cost IoT technologies**. The collected data is presented to users through a **real-time web application**.

## Features
- **Low-cost IoT hardware**
- **Wireless data transmission** over Wi-Fi
- **Data monitoring** in a web app
- **MongoDB** integration for persistent storage
- Easy firmware updates via **Arduino IDE**

---

## Hardware Components
| #  | Component | Qty | Image |
|----|-----------|-----|-------|
| 1  | ESP32-C3 SuperMini | 2 | ![ESP32-C3 Super Mini](https://github.com/PongkunSK/Smart_Insoles/blob/main/Image/esp32-c3-super-mini.webp) |
| 2  | Force Sensors | 4 | ![Force Sensor](https://github.com/PongkunSK/Smart_Insoles/blob/main/Image/FSR.jpg?raw=true) |
| 3  | Battery (50mAh, 3.7V) | 2 | ![Battery](https://github.com/PongkunSK/Smart_Insoles/blob/main/Image/battery50%20mAh.jpg?raw=true) |
| 4  | RFID Reader & Card | 1 set | ![RFID Reader](https://github.com/PongkunSK/Smart_Insoles/blob/main/Image/RFID.webp) ![RFID Card](https://github.com/PongkunSK/Smart_Insoles/blob/main/Image/RFID%20card.jpg?raw=true) |
| 5  | Insoles | 1 pair | ![Insoles](https://github.com/PongkunSK/Smart_Insoles/blob/main/Image/insoles.jpg?raw=true) |
| 6  | IR Remote & Sensor | 1 set | ![IR Remote & Sensor](https://github.com/PongkunSK/Smart_Insoles/blob/main/Image/IR%20remote%20&%20sensor.jpg?raw=true) |
| 7  | RGB LED | 1 | ![RGB LED](https://github.com/PongkunSK/Smart_Insoles/blob/main/Image/RGB%20LED.jpg?raw=true) |
| 8  | 2.2kΩ Resistors | 4 | ![2.2k Ohm Resistor](https://github.com/PongkunSK/Smart_Insoles/blob/main/Image/2.2k-ohm-resistor.jpg?raw=true) |

---

## How to Start

### 1. Wire Up the Components
![Wiring](https://github.com/PongkunSK/Smart_Insoles/blob/main/Image/Insoles%20Under%20Prototype.jpg?raw=true)

### 2. Prepare the Firmware (Arduino IDE)
1. Download the `SPPB_Test_Prototype` folder.
2. Connect each **ESP32-C3 SuperMini** board to your computer.
3. Open **Insole_Left** and **Insole_Right** sketches in Arduino IDE.
4. Go to:  
   **Tools → Board → Select "ESP32C3 Dev Module"**  
   **Tools → Port → Select your board's port** (example: `COM9` or `COM10`)
5. Upload:
   - **Insole_Left** → Left ESP32-C3 SuperMini
   - **Insole_Right** → Right ESP32-C3 SuperMini

---

## How to Use with MongoDB

This project uses **MongoDB** to store test results from the Smart Insoles.

### 1. Install MongoDB
- Download and install [MongoDB Community Edition](https://www.mongodb.com/try/download/community).
- Make sure MongoDB is running:
- 
  ```bash
  mongosh
  ```
  
- Type exit to quit.

### 2. Backend Setup (Vs Code)
1. Clone the repository and navigate to the backend folder.
2. Install dependencies:
   ```bash
    npm install
3. Create a .env file:
   
   ```ini
   MONGODB_URI=mongodb://localhost:27017/smartinsoles
   PORT=3000
   ```
   
4. Start the backend:
   
   ```bash
   node server.js
   ```
   
   You should see:
  
   ```arduino
   ✅ Connected to MongoDB
   Server running on http://localhost:3000
   ```
### 3. ESP32 Setup
- In your Arduino firmware, set the Wi-Fi credentials:
  
  ```cpp
  const char *ssid = "Your_WiFi_Name";
  const char *password = "Your_WiFi_Password";
  ```
  
- Set the backend URL:
  
  ```cpp
  const char *serverURL = "http://<backend-ip>:3000/api/test_results";
  ```
  
### 4. Testing
-  Run the backend sever.
-  Power on both ESP32 boards
-  Perform a test and check MongoDB
  
   ```bash
   mongosh
   use smartinsoles
   db.test_results.find().pretty()
   ```
   
### Data Flow Diagram

   ```css
   [ESP32 (Arduino IDE)] → HTTP POST → [Backend (VS Code / Node.js)] → [MongoDB Database]
   ```

![DataFlow](https://github.com/PongkunSK/Smart_Insoles/blob/main/Image/SmartInsoles%20Data%20Flow.png?raw=true)
### Exanple Data in MongoDB

   ```json
   {
  "userId": "insole1",
  "TUNGTresult": "P",
  "SBSTresult": "P",
  "STSTresult": "F",
  "TDSTresult": "P",
  "CSUTresult": "F",
  "timestamp": "2025-08-15T06:30:00Z"
}
   ```

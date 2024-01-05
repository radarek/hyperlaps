/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

#include <BLEDevice.h> // BLUETOOTH LIBRARY
#include <BLEUtils.h> // BLUETOOTH LIBRARY
#include <BLEServer.h> // BLUETOOTH LIBRARY
#include <BLE2902.h> // BLUETOOTH LIBRARY

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/


#define SolenoidButton 15
#define xPin 39

int ledPins[] = {32, 33, 25, 26, 27, 14, 12, 13, 19, 18};

uint8_t mapX = 0;
uint8_t score = 0;
int buttonState = 0;
uint8_t buttonValue = 0;
int ledCount = 0;
int xValue = 0;

BLEServer *MyServer = NULL;

// BLUETOOTH IDENTIFIERS - The associated motherboard has the same identifiers, that's why they can communicate with each other
// The service ID.
#define serviceID "77836fa0-2295-11eb-adc1-0242ac120002"
// The position X characteristic ID.
#define characteristicIDPositionX "7dcf58a6-2295-11eb-adc1-0242ac120002"
// The score characteristic ID.
#define characteristicIDScore "81b58170-2295-11eb-adc1-0242ac120002"
// The solenoid characteristic ID.
#define characteristicIDSolenoid "8591abe8-2295-11eb-adc1-0242ac120002"
// END BLUTOOTH IDENTIFIERS

/* Define our custom characteristic along with it's properties */
BLECharacteristic customCharacteristicPositionX(
  characteristicIDPositionX, 
  BLECharacteristic::PROPERTY_READ | 
  BLECharacteristic::PROPERTY_NOTIFY |
  BLECharacteristic::PROPERTY_WRITE
);

/* Define our custom characteristic along with it's properties */
BLECharacteristic customCharacteristicScore(
  characteristicIDScore, 
  BLECharacteristic::PROPERTY_WRITE
);

/* Define our custom characteristic along with it's properties */
BLECharacteristic customCharacteristicSolenoid(
  characteristicIDSolenoid, 
  BLECharacteristic::PROPERTY_READ | 
  BLECharacteristic::PROPERTY_NOTIFY
);

// BLUETOOTH data transfer functions
/* This function handles the server callbacks */
bool deviceConnected = false;
bool oldDeviceConnected = false;

class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* MyServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* MyServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *customCharacteristicScore) {
      uint8_t* value = customCharacteristicScore->getData();
      score = value[0];      
    }
};
// END BLUETOOTH data transfer functions

void setup() {
  //Serial.begin(19200); // Uncomment to be able to debug your code, for example to see the value sent from the X axis of the Joystick
  pinMode(SolenoidButton, INPUT);
  pinMode(xPin, INPUT);

  for (int thisLed = 0; thisLed < 10; thisLed++) {
      pinMode(ledPins[thisLed], OUTPUT);
  }

  // BLUETOOTH functions
  // Create and name the BLE Device
  BLEDevice::init("ESP-GAMEPAD-3");

  /* Create the BLE Server */
  MyServer = BLEDevice::createServer();
  MyServer->setCallbacks(new ServerCallbacks());  // Set the function that handles Server Callbacks
  
  /* Add a service to our server */
  BLEService *customService = MyServer->createService(serviceID); //  A random ID has been selected

  /* Add a characteristic to the service */
  customService->addCharacteristic(&customCharacteristicPositionX);  // customCharacteristicPositionX was defined above

  /* Add a characteristic to the service */
  customService->addCharacteristic(&customCharacteristicScore);  // customCharacteristicScore was defined above
  customCharacteristicScore.setCallbacks(new MyCallbacks()); // Enable callbacks to get back data updated from client

  /* Add a characteristic to the service */
  customService->addCharacteristic(&customCharacteristicSolenoid);  // customCharacteristicPositionX was defined above

  /* Add Descriptors to the Characteristic*/
  customCharacteristicPositionX.addDescriptor(new BLE2902());  //Add this line only if the characteristic has the Notify property

  /* Add Descriptors to the Characteristic*/
  customCharacteristicSolenoid.addDescriptor(new BLE2902());  //Add this line only if the characteristic has the Notify property

  /* Configure Advertising with the Services to be advertised */
  MyServer->getAdvertising()->addServiceUUID(serviceID);

  // Start the service
  customService->start();

  // Start the Server/Advertising
  MyServer->getAdvertising()->setMinPreferred(0x0);
  MyServer->getAdvertising()->start();
  // END BLUETOOTH functions
}

// This is the Arduino main loop function.
void loop() {
  xValue = analogRead(xPin); // IMPORTANT FOR DEBUG: xValue contains Joystick X axis values, theoretically values ​​range from 0 (left) to 4096 (right) (12bits)
  //Serial.println(xValue); // IMPORTANT FOR DEBUG: 1. It is essential to uncomment this line first to analyze the values ​​sent by the Joystick and start calibration
  
  //YOU MUST MODIFY THE FIRST TWO VALUES, EXAMPLE HERE, 0 and 4096 must be modified according to your own Joystick
  mapX = map(xValue, 0, 4096, 0, 255); // IMPORTANT FOR DEBUG: This map() function converts the 12 bits values (4096 values) ​​of xValue to transform them into 8-bit values (255 values), example here, 0 corresponds to 0 and 4096 to 255, the motherboard will only receive values ​​between 0 and 255. Why do this? Because we cannot send a 12-bit value via the BLUETOOTH of the ESP32, we can send a maximum of 8 bits  
  //Serial.println(mapX); // IMPORTANT FOR DEBUG: 2. It is essential to uncomment this line second to analyze the values ​​sent by the Joystick and start calibration. How do i calibrate: Joystick max left: 10, Joystick max right: 245. Why am I not calibrating to 0 and 255? Because if you go below 0, the value will go to 255 and if you go above 255, the value will go to 0 which will cause a movement opposite to that expected. So it's a margin of safety.
  
  buttonState = digitalRead(SolenoidButton); // buttonState contains the Solenoid Button value
  //Serial.println(buttonState); // IMPORTANT FOR DEBUG: Uncomment this line if you have a problem with the Solenoid Button
  
    // If Gamepad and Motherboard are connected together
    if (deviceConnected) {
         for (int thisLed = 0; thisLed < score + 1; thisLed++) { // All LEDs are on
           digitalWrite(ledPins[thisLed], HIGH);
           ledCount = thisLed;
        }
    
        for (ledCount; ledCount < 11; ledCount++) { // Disable a led if a point is lost
           digitalWrite(ledPins[ledCount], LOW);
        }
    
        /* Set the value */
        customCharacteristicPositionX.setValue(&mapX,1);  // This is a value of a single byte, set the position x value
        customCharacteristicPositionX.notify();  // Notify the Motherboard the position x
    
        if (buttonState == HIGH) { // If Solenoid button is pressed
          buttonValue = 1;       
        } else {
          buttonValue = 0;
        }   
    
        customCharacteristicSolenoid.setValue(&buttonValue,1);  // This is a value of a single byte, set Solenoid Button value
        customCharacteristicSolenoid.notify();  // Notify the Motherboard the Solenoid Button Value
        delay(40); // Bluetooth stack will go into congestion, if too many packets are sent
    }
    
    // Disconnected with Motherboard
    if (!deviceConnected && oldDeviceConnected) {
        for (int thisLed = 0; thisLed < 10; thisLed++) {
         digitalWrite(ledPins[thisLed], LOW); // All LEDs are off
        }
        delay(2000); // Give the bluetooth stack the chance to get things ready
        MyServer->startAdvertising(); // Restart advertising, trying to reconnect again
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
    }
}

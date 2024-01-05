#include "BLEDevice.h" // Bluetooth libray
#include <Stepper.h> // Libray to control unipolar or bipolar stepper motors

#define enablePin 12 // enablePin allows to activate or deactivate the motor, for example, the motor is activated only when you move the joystick
#define stepPin 14 
#define dirPin 15
#define limitSwitchLeft 16
#define limitSwitchRight 17
#define ledSensor 18
#define solenoidPin 32

const int stepsPerRevolution = 200;  // One revolution = 360° so step angle is 1.8⁰, 1.8 x 200 = 360. More steps = More precision
int xPosition = 115;
uint8_t score = 10; // Default number of points
uint8_t newScore = 10;
int solenoidValue = 0;
int solenoidState = 1;

TaskHandle_t xHandle = NULL;

// initialize stepper library
Stepper stepper(stepsPerRevolution, stepPin, dirPin);

// BLUETOOTH IDENTIFIERS - The associated gamepad has the same identifiers, that's why they can communicate with each other
// The service ID.
static BLEUUID serviceUUID("cfebce48-4b76-4ad9-af3f-83afc1286cb8");
// The position X characteristic ID.
static BLEUUID    charPositionXUUID("f1bfd408-2e6a-11eb-adc1-0242ac120002");
// The score characteristic ID.
static BLEUUID    charScoreUUID("f62e94a2-2e6a-11eb-adc1-0242ac120002");
// The solenoid characteristic ID.
static BLEUUID    charSolenoidUUID("fb46764e-2e6a-11eb-adc1-0242ac120002");
// END BLUTOOTH IDENTIFIERS

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristicPositionX;
static BLERemoteCharacteristic* pRemoteCharacteristicScore;
static BLERemoteCharacteristic* pRemoteCharacteristicSolenoid;
static BLEAdvertisedDevice* myDevice;

// BLUETOOTH data transfer functions
static void notifyCallbackPositionX(
  BLERemoteCharacteristic* pBLERemoteCharacteristicPositionX,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    for (int i = 0; i < length; i++)
    {
      xPosition = pData[i];
    }
}

static void notifyCallbackSolenoid(
  BLERemoteCharacteristic* pBLERemoteCharacteristicSolenoid,
  uint8_t* pDataSolenoid,
  size_t length,
  bool isNotify) {
    for (int i = 0; i < length; i++)
    {
      solenoidValue = pDataSolenoid[i];
    }
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
  }
};

bool connectToServer() {
    
    BLEClient*  pClient  = BLEDevice::createClient();

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      pClient->disconnect();
      return false;
    }


    // Obtain a reference to the characteristic POSITION_X in the service of the remote BLE server.
    pRemoteCharacteristicPositionX = pRemoteService->getCharacteristic(charPositionXUUID);
    if (pRemoteCharacteristicPositionX == nullptr) {
      pClient->disconnect();
      return false;
    }

    // Obtain a reference to the characteristic SCORE in the service of the remote BLE server.
    pRemoteCharacteristicScore = pRemoteService->getCharacteristic(charScoreUUID);
    if (pRemoteCharacteristicScore == nullptr) {
      pClient->disconnect();
      return false;
    }

    // Obtain a reference to the characteristic POSITION_X in the service of the remote BLE server.
    pRemoteCharacteristicSolenoid = pRemoteService->getCharacteristic(charSolenoidUUID);
    if (pRemoteCharacteristicSolenoid == nullptr) {
      pClient->disconnect();
      return false;
    }

    // Read the value of the characteristic.
    if(pRemoteCharacteristicPositionX->canRead()) {
      std::string value = pRemoteCharacteristicPositionX->readValue();
    }

    if(pRemoteCharacteristicPositionX->canNotify())
      pRemoteCharacteristicPositionX->registerForNotify(notifyCallbackPositionX);

    // Read the value of the characteristic.
    if(pRemoteCharacteristicSolenoid->canRead()) {
      std::string value = pRemoteCharacteristicSolenoid->readValue();
    }

    if(pRemoteCharacteristicSolenoid->canNotify())
      pRemoteCharacteristicSolenoid->registerForNotify(notifyCallbackSolenoid);

    connected = true;
    return true;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks
// END BLUETOOTH data transfer functions

// This function is executed when the joystick moves. Short explanation: a joystick permanently sends a value between 0 and 255 depending on its location.
void moveXAxis(void * parameter){
  for(;;){
      digitalWrite(enablePin, HIGH); // Motor disabled

        // If the joystick sends a value between 100 and 130, it means that it is in the center and does not move, the motor are therefore stopped
        if ((xPosition > 100) && (xPosition < 130)){ // If the motor trigger on their own without touching the joystick it means that the joystick send values lower than 100 and greater than 130 when not touching it, this is a joystick issue, open the gamepad code and monitor the values of the joystick.
          digitalWrite(stepPin, LOW);
          digitalWrite(enablePin, HIGH); // Motor disabled
        }

        // If the joystick sends a value greater than 130 then it means it has been moved to one side, the motor are therefore activated
        while (xPosition >= 130) {        
            int speed_ = map(xPosition, 130, 230, 900, 1600); // This means that between the value 130 and 230 sent by the joystick, the speed will go from 900 to 1600. If for example the joystick is moved very slightly, the speed will be 900, if on the other hand the joystick is moved to the maximum, the speed will be 1600. If your motor stalls, the last thing to try is to lower the maximum speed to 1500 for example but this is not a good solution because this side will go slower than the others, try all the other solutions before or change the motor if it is faulty. 
            stepper.setSpeed(speed_);
             
            if (digitalRead(limitSwitchLeft) == HIGH){ // If limit switch left is not triggered
              digitalWrite(enablePin, LOW); // Motor enabled
              stepper.step(-1); // Move in first direction
            } else { // If limit switch left is triggered
              digitalWrite(enablePin, HIGH); // Motor disabled - Can so only move in other direction
            }        
        }

        // If the joystick sends a value lower than 100 then it means it has been moved to the other side, the motor are therefore activated
        while (xPosition <= 100){
            int speed_ = map(xPosition, 100, 20, 900, 1600); // This means that between the value 100 and 20 sent by the joystick, the speed will go from 900 to 1600. If for example the joystick is moved very slightly, the speed will be 900, if on the other hand the joystick is moved to the maximum, the speed will be 1600. If your motor stalls, the last thing to try is to lower the maximum speed to 1500 for example but this is not a good solution because this side will go slower than the others, try all the other solutions before or change the motor if it is faulty.
            stepper.setSpeed(speed_);
  
            if (digitalRead(limitSwitchRight) == HIGH){ // If limit switch right is not triggered
              digitalWrite(enablePin, LOW); // Motor enabled
              stepper.step(1); // Move in second direction
            } else { // If limit switch right is triggered
              digitalWrite(enablePin, HIGH); // Motor disabled - Can so only move in other direction
            }          
        }
  }
}

void setup() {
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  pinMode(limitSwitchLeft, INPUT_PULLUP);
  pinMode(limitSwitchRight, INPUT_PULLUP);
  pinMode(solenoidPin, OUTPUT);
  pinMode(ledSensor, INPUT);
  digitalWrite(enablePin, HIGH);
      
  BLEDevice::init("");

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(0, false);

 // The moveXAxis function is started in multitasking, not in the main loop() function
  xTaskCreate(
    moveXAxis,      // Function that should be called
    "move X axis",    // Name of the task (for debugging)
    1000,               // Stack size (bytes)
    NULL,               // Parameter to pass
    1,                  // Task priority
    &xHandle              // Task handle
  );
} // End of setup.


// This is the Arduino main loop function.
void loop() {

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  
 if (doConnect == true) {
    if (connectToServer()) {
      pRemoteCharacteristicScore->writeValue(score);
    }
    doConnect = false;
  } else {
    // If Gamepad and Motherboard are connected together
    if (connected) {

      if (score >= 1) { // If the player has not lost the game
        int detected = digitalRead(ledSensor); // Contains the value of the receiver laser 
        
        if(solenoidValue == 1){ // If the button to trigger the solenoid is pressed on the gamepad
        
          if(solenoidState == 1){ // This prevents from keeping the button pressed, it would heat up the solenoid
             digitalWrite(solenoidPin, HIGH); // The solenoid is on
             delay(80); // Changing this delay can vary the hitting power
             digitalWrite(solenoidPin, LOW); // The solenoid is off
          }
          solenoidState = 0; // The solenoid has been activated, its state is set to 0, the state will return to 1 when the button is no longer pressed             
        } else {
          solenoidState = 1; // The button to trigger the solenoid is not pressed, the state is 1 wich means "Ready"
          digitalWrite(solenoidPin, LOW); // The solenoid is off
        }
    
        if (detected == 0) { // If the laser beam has been deflected by an obstacle
          newScore = score - 1; // The player loses a point
          pRemoteCharacteristicScore->writeValue(score - 1); // The score is sent to the gamepad to turn off a led

          if (newScore >= 1){ // If the player has not lost
            delay(700); // The laser receiver is stopped for 700ms, this avoids losing several points at once if, for example, the ball has crossed the laser beam at low speed
          }
        }
  
        score = newScore;
      }
  
    }else if(doScan){
      BLEDevice::getScan()->start(50);  // This try to reconnect the bluetooth if disconnected
    }
  }

  delay(10); // All the loop function has a slight delay otherwise it prevents the bluetooth from connecting, you can try to increase this value if your blutooth gamepad does not connect to the motherboard
} // End of loop

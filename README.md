### Hyperlaps

Project website: https://hyperlaps.io/


### Installing arduiono-cli
https://arduino.github.io/arduino-cli/0.35/installation/

### Compiling and uploading

Motherboards
```
arduino-cli core install esp32:esp32
arduino-cli lib install Stepper
arduino-cli board list

arduino-cli compile --fqbn esp32:esp32:esp32 --build-property build.extra_flags=-DMOTHERBOARD_DEFS_FILE=motherboard1.h code/Motherboard
arduino-cli upload --port /dev/cu.usbserial-0001 --fqbn esp32:esp32:esp32 code/Motherboard

arduino-cli compile --fqbn esp32:esp32:esp32 --build-property build.extra_flags=-DMOTHERBOARD_DEFS_FILE=motherboard2.h code/Motherboard
arduino-cli upload --port /dev/cu.usbserial-0001 --fqbn esp32:esp32:esp32 code/Motherboard

arduino-cli compile --fqbn esp32:esp32:esp32 --build-property build.extra_flags=-DMOTHERBOARD_DEFS_FILE=motherboard3.h code/Motherboard
arduino-cli upload --port /dev/cu.usbserial-0001 --fqbn esp32:esp32:esp32 code/Motherboard

arduino-cli compile --fqbn esp32:esp32:esp32 --build-property build.extra_flags=-DMOTHERBOARD_DEFS_FILE=motherboard4.h code/Motherboard
arduino-cli upload --port /dev/cu.usbserial-0001 --fqbn esp32:esp32:esp32 code/Motherboard
```

Gamepads
```
arduino-cli compile --fqbn esp32:esp32:esp32 --build-property build.extra_flags=-DGAMEPAD_DEFS_FILE=gamepad1.h code/Gamepad
arduino-cli upload --port /dev/cu.usbserial-0001 --fqbn esp32:esp32:esp32 code/Gamepad

arduino-cli compile --fqbn esp32:esp32:esp32 --build-property build.extra_flags=-DGAMEPAD_DEFS_FILE=gamepad2.h code/Gamepad
arduino-cli upload --port /dev/cu.usbserial-0001 --fqbn esp32:esp32:esp32 code/Gamepad

arduino-cli compile --fqbn esp32:esp32:esp32 --build-property build.extra_flags=-DGAMEPAD_DEFS_FILE=gamepad3.h code/Gamepad
arduino-cli upload --port /dev/cu.usbserial-0001 --fqbn esp32:esp32:esp32 code/Gamepad

arduino-cli compile --fqbn esp32:esp32:esp32 --build-property build.extra_flags=-DGAMEPAD_DEFS_FILE=gamepad4.h code/Gamepad
arduino-cli upload --port /dev/cu.usbserial-0001 --fqbn esp32:esp32:esp32 code/Gamepad
```


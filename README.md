# ESP-RCLink
ESP-NOW based remote control setup

This is alpha version with few missing features and requires refactoring.

Currently I use ESP32 as a controller and ESP8266 as a receiver, both NodeMCU.

The receiver is a slave device in any way possible. It gets its pin assigment from the master, outputs, inputs and everything - you flash the Arduino sketch once and everything else is set from the controller once they communicate with each other.

What works:
- Send commands from the controller to the receiver
  - Servo, analogWrite-PWM, digitalWrite, predefined blink modes for night time position announcement
- Get battery reading and signal quality from the slave to the receiver
- Sony PS2 joystick as an actual controller
- 100Hz refresh rate (Can be brough up to 200, but why?)
- Safety. After 5s timeout the receiver goes in failsafe mode and disables all the pins until communication is restored.
- HotSpot and WebServer for preview of the parameters such as battery and link quality.

Future plans:
- Better UI of the WebServer, and hopefully setup through it.
  - Web panel joystick? Will it work OK?
- Better HotSpot control, currently everything is hard coded
- Encryption? Or at least some error checking...
- Pairing procedure, currently hard coded MAC addresses of the controller and receiver.
- Over-The-Air update
- Stepper motor control on the receiver? Is it needed?
- ESP32 as a receiver? Currently no Servo.h library available. Possible:
  - Flight controller with Gyros and stuff?
  - Camera feed in browser?
  - Remote speaker on receiver end?
  - Crazy WS2182B LED drone lightshow?
- ...

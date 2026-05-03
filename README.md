## PöNG 32x32

**PöNG 32x32** is a project that serves as a platform for implementing various games.

The key components include:

- ESP32 controller
- A display based on a 32 × 32 RGB LED matrix,
- Two joysticks for player input,
- Multiple displays,
- A matrix display for showing game status information,
- A 16×2 LCD display with a rotary encoder for navigating the system menu.

Additionally, an SD card reader is integrated, which can be used, for example, to display graphics.

The project consists of documentation for:

- Hardware,
- 3D models for the casing and individual components,
- The actual software, as well as several smaller test programs.

Inspired by the classic game Pong, I implemented a similar game to demonstrate the capabilities of the platform - **PöNG 32x32**.

### Game Operating Instructions

When the device is turned on, it is in *"demo mode"*. The demo mode can be exited by pressing the encoder for approximately 2 seconds. 
The game then begins, and the device enters *"round mode"*. 
Each goal scored counts as one point.

The "round mode" can also be exited by pressing the encoder again. The device will then return to "demo mode", and the next game can be started.
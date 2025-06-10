# E-Textile-Final
For fully formatted version: https://docs.google.com/document/d/1ITf9gF5SsOBAmgqFdm8_8Y8wQJNKP5p5dcAvn7UFDFI/edit?usp=sharing

Pedometer Sock Tutorial
Use Case
The primary use case of this project is just to have a sillier way of counting steps, mainly for light indoor exercise or walking around on a nice sunny day. You may want to make this project if you’d like a silly way of encouraging yourself to do a light workout!
Supplies
7 Segment 4 Digit Display with I2C Controller link
Adafruit Flora link
Wire (I just used some stripped out of old CAT 4 cables)
Soldering equipment (Soldering iron, flux, solder, etc.)
Felt
Crew Sock
Tape Measure
LED (can be replaced with a resistor, if wanted) link
Velcro Stickies link
Washi Tape link
Jewelers Pliers link
Wirecutter link
Wirestripper link
Assembly
The Screen
If your screen did not come with an I2C controller soldered already, follow these steps to put it together:
*TODO*
Putting Together the Fabric
With the sock on, wrap a piece of felt around half of your calf and using your thumb to mark it, cut out a piece of felt that width.

With the sock off, shorten the piece of felt to be the same as the top of the sock

Sew conductive snaps into the sock, using non-conductive thread on the top, and conductive thread at the base. Using Conductive thread on the lower ones ensures that when we sew conductive thread through them later, it connects properly

Measure the distance between the two snaps at the base while the sock is on, and cut diagonally up the felt so the bottom of the felt is that length.

Sew conductive snaps into the felt piece, conductive thread at the bottom, non-conductive at the top, just like on the sock. 
Putting Together the Pressure Sensor:
Make two small squares of fabric

Sew conductive thread in a zigzag pattern across the square as shown leaving thread tails on one end.

Sew the two pieces together most of the way, with the large runs of conductive thread facing each other, leaving a large enough hole in it to add batting 

Hook up the sensor to the microcontroller and LED as shown and using this provided code, add enough batting such that it creates a range of values while pressed and released. Make sure to enable the serial monitor using ctrl+shift+M.


Finish sewing up the sensor, but leave the tails on the conductive thread
Attaching the Pressure Sensor to the Sock
Put the sock on and use washi tape to affix the sensor at the bottom of the heel. 
Sew conductive thread through one of the snaps ending the run beside the sensor, but don’t tie it off or trim it.

Tie one tail from the sensor with the tail of the tread you just sewed.
Repeat 2 and 3 with the other snap and sensor tail.
 Use Non-conductive thread to sew the pressure sensor in place
Putting Together the Electronics

Solder short wires between the I2C connections on the screen to the corresponding pins on the microcontroller
Add velcro to the back of the screen and position it at the top of the panel
Sew from 3.3V to the right most snap.
Sew from GND to the negative end of an LED.
Sew from the positive end of the LED to the leftmost conductive snap.
Sew from 10 to about halfway between the positive end of the LED and the conductive snap making sure to wrap around the thread a few times to ensure a good connection.
Sew from pin 1 to one pad on a button.
Sew from GND to the other pad on the button.


Code
Declarations
Here are the includes and declarations
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
Above are the includes for using the Adafruit LED 7 Segment Display.
#define DISPLAY_ADDRESS 0x70
Adafruit_7segment clockDisplay = Adafruit_7segment();
Here we define the address the display will communicate on, along with creating an Adafruit_7segment variable to write to.
const int pressurePin = 10;
const int buttonPin = 1;
These are the two pins for the sensors being used in this project. They are const restricts them from being changed accidently later in the program.
int steps = 0;
This is the variable for holding the number of steps taken
bool footDown = false;
float smoothed = 0;
const float alpha = 0.8;
unsigned long lastStepTime = 0;
const int stepDelay = 400; // debounce
These variables are used to aid in counting steps, defining how we smooth the input from the sensor, and how long we wait before registering another step, both to avoid counting extra steps.
// rolling buffer
const int bufferSize = 40;
int pressureBuffer[bufferSize];
int bufferIndex = 0;
bool bufferFilled = false;
These variables are used to create a rolling buffer, which allows us to re-adjust the calibration continuously as the program is run. Since the sensor is handmade, the values it outputs change overtime. 
int sensorMin = 1023;
int sensorMax = 0;
These variables hold the minimum and maximum values the sensor is currently reading.
uint8_t brightnessLevels[] = {0, 5, 10, 15};
uint8_t currentBrightnessIndex = 3;
unsigned long lastButtonPress = 0;
bool lastButtonState = HIGH;
const int buttonDelay = 200;
These variables are for changing the brightness of the screen with the pushbutton.




Setup
void setup() {
  clockDisplay.begin(DISPLAY_ADDRESS);
  pinMode(pressurePin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.begin(9600);
First we initialize the display by starting it on the address we defined earlier. Then we set the pinModes of each input, it’s not necessary to define pressurePin as an input since it’s being used for analogue reads, but it makes it easier to read. Finally we initialize our serial monitor for debugging.
  // tell user it's calibrating
  clockDisplay.writeDigitRaw(0,57); // C
  clockDisplay.writeDigitRaw(1, 119); // A
  clockDisplay.writeDigitRaw(3, 56); // L
  clockDisplay.writeDisplay();
Here we’re writing individual characters to each display digit. Each segment within a display is individually addressable by adding the value for each segment up. 
  // Initial calibration

  while (millis() < 3000) {
	int read = analogRead(pressurePin);
	pressureBuffer[bufferIndex++] = read;
	if (bufferIndex >= bufferSize) {
  	bufferIndex = 0;
  	bufferFilled = true;
	}
  }
Here we fill out our initial pressureBuffer array. This will get continuously updated as the program runs, but we need an initial run through before we start. Delay could be shorter, but it feels more like a calibration if there’s a longer delay. 

  smoothed = analogRead(pressurePin);
We’ll smooth the input later, but need to set the smoothed input to an initial read of the pressurePin. 
}

Main Loop
void loop() {
  bool buttonState = digitalRead(buttonPin);
  if (buttonState == LOW && lastButtonState == HIGH && millis() - lastButtonPress > buttonDelay) {
	lastButtonPress = millis();
	currentBrightnessIndex = (currentBrightnessIndex + 1) % (sizeof(brightnessLevels));

clockDisplay.setBrightness(brightnessLevels[currentBrightnessIndex]);
	Serial.print("Brightness level: ");
	Serial.println(brightnessLevels[currentBrightnessIndex]);
  }
  lastButtonState = buttonState;
This if statement handles the pushbutton for screen brightness. Mainly we’re checking to make sure that the current state is pushed, the state before it is not-pushed, and we want to make sure that it’s been longer since the last press than the delay. This seems overly complicated, but it ensures we don’t skip a bunch of brightnesses if the button gets pushed in a way where it clicks too fast, or is held down.

  int raw = analogRead(pressurePin);

  // Add to rolling buffer
  pressureBuffer[bufferIndex++] = raw;
  if (bufferIndex >= bufferSize) {
	bufferIndex = 0;
	bufferFilled = true;
  }
This loop fills and refills the rolling buffer. Whenever the final entry in the buffer is written, it sets the index back to 0 so all values can get rewritten.

  // Compute min and max from buffer
  int rangeLength = bufferFilled ? bufferSize : bufferIndex;
  sensorMin =1023;
  sensorMax = 0;
  for (int i = 0; i < rangeLength; i++) {
	int val = pressureBuffer[i];
	if (val < sensorMin) sensorMin = val;
	if (val > sensorMax) sensorMax = val;
  }
This is for finding the min and max of the buffer. It simply loops through the whole buffer, as long as it’s full, and finds the current min and max.

  // Apply EMA smoothing
  smoothed = alpha * raw + (1 - alpha) * smoothed;
This is where we apply Exponential Moving Average to the pressure sensor output, this ensures there aren’t huge spikes in the readings. 

  int delta = smoothed - sensorMin;
  int range = sensorMax - sensorMin;
Here we define the delta (the change between the sensors minimum and the smoothed input) and the range (the difference between sensor max and min). These are used to define what a step is, the delta being the reading we use to compare with the range, which will be used for defining the step thresholds

  // Avoid phantom steps by ignoring readings while the range is small
  if (range < 50) {
	footDown = false;  // reset foot state if range too small
	Serial.println("Range too small, ignoring steps.");
	delay(10);
	return;
  }
Here we throw out any readings of sufficiently small range. This ensures that we don’t detect steps when there aren’t any. Mainly if a user stays in one state for too long (either currently stepping or not).

  int thresholdHigh = 0.60 * range;
  int thresholdLow  = 0.25 * range;
These define the thresholds for what counts as a step, the high one is for the initial foot down reading, and the low one is for the user lifting their foot up.

  // Step detection with debounce and hysteresis
  if (!footDown && delta > thresholdHigh && millis() - lastStepTime > stepDelay) {
	steps++;
	footDown = true;
	lastStepTime = millis();
  } else if (footDown && delta < thresholdLow) {
	footDown = false;
  }
This is where steps are incremented, and where we use our thresholds and timings. All of this is to ensure that extra steps aren’t counted when there aren’t any.


Debug
All of these are simply debug print statements used to show us the current values of the most important variables. Raw sensor readings, smoothed sensor readings, delta, sensorMin, sensorMax, thresholdHigh, thresholdLow, and steps.
  // debug info
  Serial.print("Raw: ");
  Serial.print(raw);
  Serial.print(" | Smoothed: ");
  Serial.print(smoothed, 1);
  Serial.print(" | Delta: ");
  Serial.print(delta);
  Serial.print(" | Min: ");
  Serial.print(sensorMin);
  Serial.print(" | Max: ");
  Serial.print(sensorMax);
  Serial.print(" | Must exceed: ");
  Serial.print(thresholdHigh);
  Serial.print(" | Must be under: ");
  Serial.print(thresholdLow);
  Serial.print(" | Steps: ");
  Serial.println(steps);

  // update display
  clockDisplay.clear();
  clockDisplay.print(steps);
  clockDisplay.writeDisplay();
}




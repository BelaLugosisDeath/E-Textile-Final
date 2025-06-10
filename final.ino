#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

#define DISPLAY_ADDRESS 0x70
Adafruit_7segment clockDisplay = Adafruit_7segment();

const int pressurePin = 10;
const int buttonPin = 1;

int steps = 0;
bool footDown = false;

float smoothed = 0;
const float alpha = 0.8; 

unsigned long lastStepTime = 0;
const int stepDelay = 400; // debounce

// rolling buffer
const int bufferSize = 40;
int pressureBuffer[bufferSize];
int bufferIndex = 0;
bool bufferFilled = false;

int sensorMin = 1023;
int sensorMax = 0;

uint8_t brightnessLevels[] = {0, 5, 10, 15};
uint8_t currentBrightnessIndex = 3;
unsigned long lastButtonPress = 0;
bool lastButtonState = HIGH;
const int buttonDelay = 200;


void setup() {
  clockDisplay.begin(DISPLAY_ADDRESS);
  pinMode(pressurePin, INPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.begin(9600);

  // tell user it's calibrating
  clockDisplay.writeDigitRaw(0,57); // C
  clockDisplay.writeDigitRaw(1, 119); // A
  clockDisplay.writeDigitRaw(3, 56); // L
  clockDisplay.writeDisplay();

  // Initial calibration

  while (millis() < 3000) {
    int read = analogRead(pressurePin);
    pressureBuffer[bufferIndex++] = read;
    if (bufferIndex >= bufferSize) {
      bufferIndex = 0;
      bufferFilled = true;
    }
  }

  smoothed = analogRead(pressurePin);
}

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

  int raw = analogRead(pressurePin);

  // Add to rolling buffer
  pressureBuffer[bufferIndex++] = raw;
  if (bufferIndex >= bufferSize) {
    bufferIndex = 0;
    bufferFilled = true;
  }

  // Compute min and max from buffer
  int rangeLength = bufferFilled ? bufferSize : bufferIndex;
  sensorMin =1023;
  sensorMax = 0;
  for (int i = 0; i < rangeLength; i++) {
    int val = pressureBuffer[i];
    if (val < sensorMin) sensorMin = val;
    if (val > sensorMax) sensorMax = val;
  }

  // Apply EMA smoothing
  smoothed = alpha * raw + (1 - alpha) * smoothed;

  int delta = smoothed - sensorMin;
  int range = sensorMax - sensorMin;

  // Avoid phantom steps by ignoring readings while the range is small
  if (range < 50) {
    footDown = false;  // reset foot state if range too small
    // Serial print to show ignoring small range (optional)
    Serial.println("Range too small, ignoring steps.");
    delay(10);
    return;
  }

  int thresholdHigh = 0.60 * range;
  int thresholdLow  = 0.25 * range;

  // Step detection with debounce and hysteresis
  if (!footDown && delta > thresholdHigh && millis() - lastStepTime > stepDelay) {
    steps++;
    footDown = true;
    lastStepTime = millis();
  } else if (footDown && delta < thresholdLow) {
    footDown = false;
  }

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

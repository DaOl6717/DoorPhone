#include "esp32-hal-gpio.h"
#include <Arduino.h>
#include <cstdlib>
#include <math.h>

int corrected_brightness(int);

const int photo_pin = 34;
int threshold = 500;

void setup() {
  Serial.begin(115200);
  pinMode(photo_pin, INPUT);
}

void loop() {
  int pin_value = analogRead(photo_pin);
  int brightness = corrected_brightness(pin_value);
  Serial.print("Brightness: ");
  Serial.println(brightness);

  if (brightness > threshold) {
    Serial.println("Wobalobaloo");
  }
  delay(1000);
}

int corrected_brightness(int x) {
  return abs(x - 4095);
}
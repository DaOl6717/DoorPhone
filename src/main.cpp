#include "esp32-hal-gpio.h"
#include <Arduino.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>


int corrected_brightness(int);
void press_button(int, int);
void callback(char*, byte*, uint8_t); // Om uint8_t it funkar, prova unsigned int

// WiFi Settings
constexpr char* SSID = "YOUR_SSID";
constexpr char* PSSWD = "YOUR_PASSWORD";

// MQTT Settings
char *MQTT_SERVER ="broker.yourserver.com";
int MQTT_PORT = 0000;
constexpr char* CLIENT_NAME = "DOORPHONE";
constexpr char* PUB_TOPIC = ""; // The topic the client will publish to
constexpr char* SUB_TOPIC ="";  // The topic the client will subscribe to

WiFiClient client;                // Create WiFi object
PubSubClient mqttClient(client);  // Create MQTT Client object
Servo button_servo;               // Create servo object

constexpr int PHOTO_PIN = 34;     // GPIO pin for photo resistor
constexpr int SERVO_PIN = 0;      // GPIO pin for servo
constexpr int FREQ = 50;          // Servo frequency, 50Hz standard
constexpr int MIN_US = 1000;      // Minimum servo pulse width in µs
constexpr int MAX_US = 2000;      // Maximum servo pulse width in µs
constexpr int THRESHOLD = 500;    // Brightness threshold
constexpr int DELAY_MS = 200;     // Servo delay in ms
constexpr int START_ANGLE = 90;   // Initial servo angle
constexpr int ANGLE = 20;         // Angle needed to press one button
constexpr int ADC_MAX = 4095;  // Max value from analogue-to-digital converter

bool already_pressed = false;     // Prevent repeated button presses

void setup() {
  Serial.begin(115200);
  pinMode(PHOTO_PIN, INPUT);
  
  WiFi.mode(WIFI_STA);  // Set WiFi to station mode
  WiFi.begin(SSID, PSSWD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("WiFi connected to: "); Serial.println(SSID);
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  delay(2000);
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(callback);

  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  button_servo.setPeriodHertz(FREQ);
  // Default min/max of 1000us/2000us
  // Might need adjustment to increase accuracy
  button_servo.attach(SERVO_PIN, 1000, 2000);

  button_servo.write(START_ANGLE);  // Set servo to starting position
}

void loop() {
  int pin_value = analogRead(PHOTO_PIN);  // Read photo resistor value
  // Convert raw ADC value so higher numbers represent more light
  int brightness = corrected_brightness(pin_value);

  // Debug prints, TODO: remove when done
  Serial.print("Brightness: ");
  Serial.println(brightness);

  // When door phone lights up (rings)
  if (brightness > THRESHOLD && !already_pressed)
  {
    press_button(ANGLE, DELAY_MS);
    already_pressed = true;
  }
  else if (brightness <= THRESHOLD)
  {
    already_pressed = false;
  }
  delay(1000);
}

/*
  Takes the raw brightness value of a photo resistor to make a higher value
  represent an increased brightness and lower value a decreased brightness.

  @param raw_value Value read from the photo resistor
  @return Converted brightness value
*/
int corrected_brightness(int raw_value) {
  return abs(raw_value - ADC_MAX);
}

/*
  Controls the servo to press the upper button on the door phone, then
  turn and press the lower button.

  @note Assumes the same model of Assa Aptus door phone as I have.
  @param angle: how much the servo should turn to press a button
  @param delay_ms: how long the servo should wait after turning
*/
void press_button(int angle, int delay_ms)
{
  button_servo.write(START_ANGLE + ANGLE);
  delay(DELAY_MS);
  button_servo.write(START_ANGLE - ANGLE);
  delay(DELAY_MS);
  button_servo.write(START_ANGLE);
}

void callback(char* topic, byte* payload, uint8_t length) {
  Serial.println("Message received from: ");
  Serial.println(topic);

  for (uint8_t i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }

  Serial.println();
  Serial.println();
}
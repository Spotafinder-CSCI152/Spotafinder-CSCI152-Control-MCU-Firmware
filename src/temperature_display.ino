/*
 * Project temperature_display
 * Description:
 * Author:
 * Date:
 */
//

#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "SHT15.h"

// pin mapping for Particle Argon
#define OLED_MOSI D6
#define OLED_CLK D7
#define OLED_DC D8
#define OLED_CS D4
#define OLED_RESET D5

#define SetPointPin A4
#define ModePin A5
#define FanPin  A3

#define RefreshRate 5000

Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
SHT15 sensorSHT15(D9, D10);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

int tempSetpoint;
int tempSetpointAnalog;
int mode = 0;
int val;
int status = 0;
double temperature;
unsigned long oldDelayTime = 0; 

void setup() {

  WiFi.setCredentials("MASTER_LAPTOP", "passwrd100");
  WiFi.setCredentials("ATT8Vuw69R", "4t565w3ufx2r");
  Particle.variable("temp", temperature);
  Particle.variable("stat", status);
  Particle.variable("mode", mode);
  Particle.function("modeC", modeControl);

  Serial.begin(9600);

  display.begin(SSD1306_SWITCHCAPVCC);

  /// display startup
  display.clearDisplay();
  /// text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("Initialization...");
  display.display();

  pinMode(ModePin, INPUT);
  pinMode(FanPin, OUTPUT);
}

void loop() {

  unsigned long currentDelayTime = millis();
  if (currentDelayTime - oldDelayTime >= RefreshRate) {
    oldDelayTime = currentDelayTime;

    sensorSHT15.readSensor();
  }

  tempSetpointAnalog = analogRead(SetPointPin);  // get setpoint for temp
  tempSetpoint = map(tempSetpointAnalog, 0, 4095, 75, 90);

  
  temperature = sensorSHT15.getTemperatureF();  // get tempF
  displayTemperature(temperature);              // display tempF

  val = digitalRead(ModePin);  // change mode
  if (val) {
    mode += 1;
    mode = mode % 3;
  }
  while (val) {
    val = digitalRead(ModePin);
  }
  controlFan();
  delay(10);
}


int modeControl(String command) {
  if (command == "0") {
    mode = 0;
    return 1;
  } else if (command == "1") {
    mode = 1;
    return 1;
  } else if (command == "2") {
    mode = 2;
    return 1;
  } else {
    return -1;
  }
}

void controlFan() {
  if (mode == 0) {  // off
    digitalWrite(FanPin, 0);
    status = 0;
  }
  if (mode == 1) {  // on
    digitalWrite(FanPin, 1);
    status = 1;
  }
  if (mode == 2) {  // auto

    if (abs(tempSetpoint - (int)temperature) >=
        1) {  // if temp is off by 1 deg then
      if ((int)temperature > tempSetpoint &&
          status == 1) {  // if temp too high && fan on

        digitalWrite(FanPin, 0);  // turn off fan
        status = 0;
      }
      if ((int)temperature < tempSetpoint &&
          status == 0) {  // if temp too low && fan off

        digitalWrite(FanPin, 1);  // turn on fan
        status = 1;
      }
    }
  }
}

void displayTemperature(double temperature) {
  display.clearDisplay();
  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print("Temperature: ");
  display.print(temperature);
  display.println(" F");
  display.println("");
  display.print("Target Temp: ");
  display.println(tempSetpoint);
  display.println("");
  display.print("Mode: ");

  if (mode == 0) display.print("Off");
  if (mode == 1) display.print("On");
  if (mode == 2) display.print("Auto");

  display.display();
}

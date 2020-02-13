/*
 * Project temperature_display
 * Description:
 * Author:
 * Date:
 */
//
//CURRENT CODE TARGETS PARTICLE CORE FIRMWARE V1.0.1
//Use CLI to compile and flash
 #include "Adafruit_GFX.h"
 #include "Adafruit_SSD1306.h"

 //SYSTEM_MODE(SEMI_AUTOMATIC);
 //Wifi.off();

 #define OLED_MOSI   D2
 #define OLED_CLK    D3
 #define OLED_DC     D4
 #define OLED_CS     D5
 #define OLED_RESET  D6
 Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

 #define NUMFLAKES 10
 #define XPOS 0
 #define YPOS 1
 #define DELTAY 2


 int clkPin = D1;  //core D1 is clock
 int dataPin = D0; //
 double temperature;

 int tempSetpoint;
 int tempSetpointAnalog;
 int mode = 0;
 int val;
 int status = 0;
 int modeControl(String command);

void setup() {

  WiFi.setCredentials("MASTER_LAPTOP", "passwrd100");
  WiFi.setCredentials("ATT8Vuw69R", "4t565w3ufx2r");
  Particle.variable("temp", temperature);
  Particle.variable("stat", status);
  Particle.variable("mode", mode);

  Particle.function("modeC", modeControl);

  Serial.begin(9600);

  display.begin(SSD1306_SWITCHCAPVCC);

  ///display startup
  display.clearDisplay();
  ///text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("Initialization...");
  display.display();

  delay(500);

  //pin initialization
  pinMode(clkPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  digitalWrite(dataPin,HIGH);		//#dataPin = 1
  digitalWrite(clkPin,LOW);		//clkPin  = 0

  pinMode(A4, INPUT);// button
  pinMode(D7, OUTPUT);// led and relay switch
}



void loop() {

	tempSetpointAnalog = analogRead(A6);//get setpoint for temp  // core is A6
	tempSetpoint = map(tempSetpointAnalog, 0, 4095, 75, 90);

	temperature = getTemperatureF();//get temp and convert
  displayTemperature(temperature);// display temp

	val = digitalRead(A4);//change mode
	if(val){
		mode += 1;
		mode = mode % 3;
	}
	while(val){
    val = digitalRead(A4);
	}
	controlFan();
	delay(10);

}

/*
int modeControl(String command){
    mode += 1;
	mode = mode % 3;
    return 1;
}
*/


int modeControl(String command){
        if(command == "zero"){
            mode = 0;
            return 1;
        }
        else if(command == "one"){
            mode = 1;
            return 1;
        }
        else if(command == "two"){
            mode = 2;
            return 1;
        }
        else{
            return -1;
        }
}



void controlFan(){
	if(mode == 0){//off
		digitalWrite(D7,0);
		status = 0;
	}
	if(mode == 1){//on
		digitalWrite(D7,1);
		status = 1;
	}
	if(mode == 2){//auto

		if(abs(tempSetpoint - (int)temperature) >= 1){//if temp is off by 1 deg then
			if((int)temperature > tempSetpoint && status == 1){//if temp too high && fan on

    /*if(abs((double)tempSetpoint - temperature) >= 1.5){//if temp is off by 1.5 deg then
      if(temperature > (double)tempSetpoint && status == 1){//if temp too high && fan on
      */
				digitalWrite(D7,0);//turn off fan
				status = 0;
			}
			if((int)temperature < tempSetpoint && status == 0){//if temp too low && fan off

      //if(temperature < (double)tempSetpoint && status == 0){//if temp too low && fan off

				digitalWrite(D7,1);//turn on fan
				status = 1;
			}
		}
	}

}



double  getTemperatureF(){
	double data;
	data = readTemperatureRaw();
    temperature = (data * 0.018) - 39.4;  //current temperature in degrees Fahrenheit
	return temperature;
}


void displayTemperature(double temperature){

  display.clearDisplay();
  //text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("Temperature: ");
  display.print(temperature);
  display.println(" F");
  display.println("");
  display.print("Target Temp: ");
  display.println(tempSetpoint);
  display.println("");
  display.print("Mode: ");

  if(mode == 0)
	display.print("Off");
  if(mode == 1)
	display.print("On");
  if(mode == 2)
	display.print("Auto");

  display.display();
}



double readTemperatureRaw(){
  int val;

  // Command to send to the SHT1x to request Temperature
  int _gTempCmd  = 0b00000011;

  sendCommandSHT(_gTempCmd, dataPin, clkPin);

  waitForResultSHT(dataPin);

  val = getData16SHT(dataPin, clkPin);

  skipCrcSHT(dataPin, clkPin);

  return (val);
}



int myShiftIn(int _dataPin, int _clockPin, int _numBits)// commands for reading/sending data to a SHTx sensor
{
  int ret = 0;
  int i;

  for (i=0; i<_numBits; ++i)
  {
     digitalWrite(_clockPin, HIGH);
     delay(10);  // I don't know why I need this, but without it I don't get my 8 lsb of temp
     ret = ret*2 + digitalRead(_dataPin);
     digitalWrite(_clockPin, LOW);
  }

  return(ret);
}


void sendCommandSHT(int _command, int _dataPin, int _clockPin)// send a command to the SHTx sensor
{
  int ack;

  // Transmission Start
  pinMode(_dataPin, OUTPUT);
  pinMode(_clockPin, OUTPUT);
  digitalWrite(_dataPin, HIGH);
  digitalWrite(_clockPin, HIGH);
  digitalWrite(_dataPin, LOW);
  digitalWrite(_clockPin, LOW);
  digitalWrite(_clockPin, HIGH);
  digitalWrite(_dataPin, HIGH);
  digitalWrite(_clockPin, LOW);

  // The command (3 msb are address and must be 000, and last 5 bits are command)
  shiftOut(_dataPin, _clockPin, MSBFIRST, _command);

  // Verify we get the correct ack
  digitalWrite(_clockPin, HIGH);
  pinMode(_dataPin, INPUT);
  ack = digitalRead(_dataPin);
  if (ack != LOW) {
    //Serial.println("Ack Error 0");
  }
  digitalWrite(_clockPin, LOW);
  ack = digitalRead(_dataPin);
  if (ack != HIGH) {
    //Serial.println("Ack Error 1");
  }
}


void waitForResultSHT(int _dataPin)// wait for the SHTx answer
{
  int i;
  int ack;

  pinMode(_dataPin, INPUT);

  for(i= 0; i < 100; ++i)
  {
    delay(10);
    ack = digitalRead(_dataPin);

    if (ack == LOW) {
      break;
    }
  }

  if (ack == HIGH) {
    //Serial.println("Ack Error 2"); // Can't do serial stuff here, need another way of reporting errors
  }
}


int getData16SHT(int _dataPin, int _clockPin) // get data from the SHTx sensor
{
  int val;

  // get the MSB (most significant bits)
  pinMode(_dataPin, INPUT);
  pinMode(_clockPin, OUTPUT);
  val = myShiftIn(_dataPin, _clockPin, 8);
  val *= 256; // this is equivalent to val << 8;

  // send the required ACK
  pinMode(_dataPin, OUTPUT);
  digitalWrite(_dataPin, HIGH);
  digitalWrite(_dataPin, LOW);
  digitalWrite(_clockPin, HIGH);
  digitalWrite(_clockPin, LOW);

  // get the LSB (less significant bits)
  pinMode(_dataPin, INPUT);
  val |= myShiftIn(_dataPin, _clockPin, 8);

  return val;
}

void skipCrcSHT(int _dataPin, int _clockPin)
{
  // Skip acknowledge to end trans (no CRC)
  pinMode(_dataPin, OUTPUT);
  pinMode(_clockPin, OUTPUT);

  digitalWrite(_dataPin, HIGH);
  digitalWrite(_clockPin, HIGH);
  digitalWrite(_clockPin, LOW);
}

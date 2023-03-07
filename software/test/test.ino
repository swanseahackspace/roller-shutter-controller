/* Roller Shutter Controller - Test Software
  - WHAT IT DOES
   Checks the hardware is working correctly for easier debugging.
   Once the startup sequence has completed, the current uptime is shown on the screen.
   In this screen, press the button on the ESP32 module (pin 9) to toggle the relay on or off
*/

/*-----( Import needed libraries )-----*/
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/*-----( Declare Constants and Pin Numbers )-----*/
const int brightnessLED = 20; //LED PWM Value up to 255
const long debounceDelay = 500;    // the debounce time; increase if the output flickers

#define LED1 0
#define LED2 4
#define LED3 5
#define authLED 6
#define enableRelay 7
#define data0 21
#define data1 20
#define insideRaiseLogic 1
#define outsideRaiseLogic 3
#define outsideLowerLogic 2
#define button 9

static const unsigned char PROGMEM logo_bmp[] =
{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x07, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00,
  0x7f, 0xff, 0xff, 0xf8, 0x05, 0xa0, 0x1f, 0xff, 0xff, 0xf0, 0x7f, 0xfe, 0x0f, 0xff, 0xff, 0xc1,
  0xff, 0xff, 0x83, 0xff, 0xff, 0x87, 0xfe, 0x7f, 0xe1, 0xff, 0xff, 0x0f, 0xfc, 0x3f, 0xf0, 0xff,
  0xfe, 0x3f, 0xf8, 0x1f, 0xfc, 0x7f, 0xfc, 0x7f, 0xf0, 0x0f, 0xfe, 0x3f, 0xf8, 0xff, 0xe0, 0x07,
  0xfe, 0x1f, 0xf8, 0xff, 0xc0, 0x07, 0xff, 0x1f, 0xf1, 0xff, 0x80, 0x07, 0xff, 0x8f, 0xe3, 0xff,
  0x00, 0x03, 0xff, 0xc7, 0xe3, 0xff, 0x00, 0x01, 0xff, 0xc7, 0xc7, 0xfd, 0x80, 0x00, 0xff, 0xe7,
  0xc7, 0xf9, 0x80, 0x00, 0x7f, 0xe3, 0xcf, 0xf0, 0xe0, 0x00, 0x3f, 0xe3, 0xcf, 0xe0, 0x30, 0x00,
  0x17, 0xf1, 0x8f, 0xc0, 0x1f, 0x00, 0x03, 0xf1, 0x8f, 0x80, 0x1e, 0x00, 0x01, 0xf1, 0x9f, 0x00,
  0x0c, 0x00, 0x00, 0xf1, 0x8e, 0x00, 0x18, 0x00, 0x00, 0x79, 0x9c, 0x00, 0x10, 0x00, 0x00, 0x31,
  0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x88, 0x00, 0x00, 0x00, 0x00, 0x51, 0x82, 0x00, 0x00, 0x00,
  0x00, 0x01, 0x82, 0x00, 0x00, 0x10, 0x00, 0xa1, 0x84, 0x40, 0x00, 0x10, 0x00, 0x81, 0x85, 0x00,
  0x00, 0x00, 0x02, 0x23, 0xc0, 0x50, 0x00, 0x02, 0x00, 0xa3, 0xc5, 0x08, 0x00, 0x00, 0x0a, 0x03,
  0xc0, 0x52, 0x00, 0x00, 0x88, 0xa3, 0xe1, 0x12, 0x00, 0x00, 0x4a, 0x07, 0xe1, 0x24, 0x40, 0x00,
  0x10, 0x87, 0xf0, 0x91, 0x00, 0x01, 0x4a, 0x0f, 0xf8, 0x25, 0x00, 0x00, 0x10, 0x0f, 0xf8, 0x21,
  0x40, 0x05, 0x4a, 0x1f, 0xfc, 0x0a, 0x10, 0x00, 0x10, 0x3f, 0xfe, 0x02, 0x80, 0x15, 0x48, 0x7f,
  0xff, 0x08, 0x54, 0x01, 0x10, 0xff, 0xff, 0x85, 0x00, 0x54, 0x81, 0xff, 0xff, 0xc0, 0x55, 0x01,
  0x03, 0xff, 0xff, 0xe0, 0x10, 0xa8, 0x07, 0xff, 0xff, 0xf8, 0x01, 0x00, 0x1f, 0xff, 0xff, 0xfe,
  0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xc0, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

/*-----( Declare objects )-----*/
//Initialise display library with no reset pin by passing it -1
Adafruit_SSD1306 display(-1);

#if (SSD1306_LCDHEIGHT != 48)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

/*-----( Declare Variables )-----*/
int buttonState = 0; // state of on-module button
int relayState = 0;

long lastDebounceTime = 0;  // the last time the output pin was toggled


void setup() { /****** SETUP: RUNS ONCE ******/

  //setup GPIO
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(enableRelay, OUTPUT);
  pinMode(button, INPUT_PULLUP);

  Serial.begin(9600);

  // generate display voltage from 3V3 rail
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)

  display.setRotation(2);
  display.clearDisplay();
  // display init complete

  display.drawBitmap(8, 0, logo_bmp, 48, 48, 1);
  display.display();
  delay(1000);

  display.setTextSize(1);
  display.setTextColor(WHITE);

  testHardware();


} //--(end setup )---

void loop() { /****** LOOP: RUNS CONSTANTLY ******/

  buttonState = !digitalRead(button);

  //filter out any noise by setting a time buffer
  if ( (millis() - lastDebounceTime) > debounceDelay) {

    //if the button has been pressed, lets toggle the LED from "off to on" or "on to off"
    if ( (buttonState == HIGH) && (relayState == 0) ) {

      digitalWrite(enableRelay, HIGH);
      relayState = 1;
      lastDebounceTime = millis();
    }
    else if ( (buttonState == HIGH) && (relayState == 1) ) {

      digitalWrite(enableRelay, LOW);
      relayState = 0;
      lastDebounceTime = millis();
    }
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("*Uptime*");
  display.println("");
  display.print("Relay: ");
  display.println(relayState);
  display.print(uptime());
  display.display();

} //--(end main loop )---

/*-----( Declare User-written Functions )-----*/
void testHardware() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("*Testing*");

  display.print("LED1");
  display.display();
  testLED(LED1);
  display.print("LED2");
  display.display();
  testLED(LED2);
  display.print("LED3");
  display.display();
  testLED(LED3);

  delay(500);

  testRelay();

  delay(500);

}

void testLED(int pinNumber) {
  analogWrite(pinNumber, brightnessLED);
  for (int i = 0; i < 3; i++) {
    display.print(".");
    display.display();
    delay(300);
  }
  display.println("OK");
  display.display();
  analogWrite(pinNumber, 0);
  delay(200);
}

void testRelay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("*Relay*");
  display.println("");
  display.print("On");
  display.display();

  digitalWrite(enableRelay, HIGH);
  for (int i = 0; i < 4; i++) {
    display.print(".");
    display.display();
    delay(300);
  }
  display.println("OK");
  display.display();
  delay(200);

  display.print("Off");

  digitalWrite(enableRelay, LOW);
  for (int i = 0; i < 3; i++) {
    display.print(".");
    display.display();
    delay(300);
  }
  display.println("OK");
  display.display();
  delay(200);
}

char *uptime() // Function made to millis() be an optional parameter
{
  return (char *)uptime(millis()); // call original uptime function with unsigned long millis() value
}

char *uptime(unsigned long milli)
{
  static char _return[32];
  unsigned long secs = milli / 1000, mins = secs / 60;
  unsigned int hours = mins / 60, days = hours / 24;
  milli -= secs * 1000;
  secs -= mins * 60;
  mins -= hours * 60;
  hours -= days * 24;
  sprintf(_return, "%2.2d days   %2.2d:%2.2d:%2.2d", (byte)days, (byte)hours, (byte)mins, (byte)secs, (int)milli);
  return _return;
}

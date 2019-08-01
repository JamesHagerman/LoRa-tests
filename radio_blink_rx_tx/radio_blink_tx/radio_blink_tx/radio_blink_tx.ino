#include <Arduino.h>

// Radio module:
#include <SPI.h> // needed for radio frequency module
#include <RH_RF95.h> // Radio include for all lora feather9x_RX

// Adafruit_SDD1306
//#include <SPI.h> // already included because of the radio module
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#if !defined(ARDUINO_ARCH_SAM) && !defined(ARDUINO_ARCH_SAMD) && !defined(ESP8266) && !defined(ARDUINO_ARCH_STM32F2)
 #include <util/delay.h>
#endif

extern "C" char *sbrk(int i);

int FreeRam () {
  char stack_dummy = 0;
  return &stack_dummy - sbrk(0);
}

// peripherial setup:
#define VBATPIN A7
#define LEDPIN 13

// LoRa pins:
// for feather m0
//#define RFM95_CS 8 // CS MUST BE SET HIGH when not using radio! (it has no pullup built in...)
//#define RFM95_RST 4
//#define RFM95_INT 3

// For LoRa Featherwing:
#define RFM95_RST     11   // "A" wired to "RST"
#define RFM95_CS      10   // "B" wired to "CS"
#define RFM95_INT     9    // "C" wired to "IRQ"

// For LoRa Featherwing on Particle board:
//#define RFM95_RST     D6   // "A" wired to "RST"
//#define RFM95_CS      D5   // "B" wired to "CS"
//#define RFM95_INT     D4   // "C" wired to "IRQ"

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 433.0
// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// OLED Screen setup:
Adafruit_SSD1306 display = Adafruit_SSD1306();
//#define BUTTON_A 9 // Button A on pin 9 collides with the LoRa Featherwing!
#define BUTTON_B 6  
#define BUTTON_C 5
#if (SSD1306_LCDHEIGHT != 32)
  #error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

void setup() {
  // initialize digital pin 13 as an output.
  pinMode(LEDPIN, OUTPUT);

  digitalWrite(LEDPIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);              // wait for a second
  digitalWrite(LEDPIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);              // wait for a second

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  // while (!Serial); // Remove this if you don't want to wait for USB
  Serial.begin(9600);
  delay(100);

  Serial.println("Feather LoRa TX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);

  // OLED Screen setup:
  Serial.println("OLED FeatherWing test");
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  // init done
  Serial.println("OLED begun");

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  Serial.println("IO test");

//  pinMode(BUTTON_A, INPUT_PULLUP); // Button A on pin 9 collides with the LoRa Featherwing!
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.setRotation(0); //0 normal. 1 opposite usb top
  display.print("Connecting to SSID\n'adafruit':");
  display.print("connected!");
  display.println("IP: 10.0.1.23");
  display.println("Sending val #0");
  display.setCursor(0,0);
  display.display(); // actually display all of the above

}

int16_t packetnum = 0;  // packet counter, we increment per xmission
int16_t responseCount = 0;
void loop() {

  Serial.println("Sending to rf95_server");
  // Send a message to rf95_server

  char radiopacket[20] = "KM6IDA request ##";
  itoa(packetnum++, radiopacket+15, 10);
  Serial.print("Sending "); Serial.println(radiopacket);
  radiopacket[19] = 0;

  Serial.println("Sending..."); delay(10);
  rf95.send((uint8_t *)radiopacket, 20);

  Serial.println("Waiting for packet to complete..."); delay(10);
  rf95.waitPacketSent();
  // Now wait for a replyu
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);

  Serial.println("Waiting for reply..."); delay(10);
  if (rf95.waitAvailableTimeout(1000)) {
    // Should be a reply message for us now
    if (rf95.recv(buf, &len)) {
      Serial.print("Got reply: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
      responseCount++;
    } else {
      Serial.println("Receive failed");
    }
  } else {
    Serial.println("No reply, is there a listener around?");
  }
//  delay(1000);

   digitalWrite(LEDPIN, HIGH);   // turn the LED on (HIGH is the voltage level)
   delay(1000);              // wait for a second
   digitalWrite(LEDPIN, LOW);    // turn the LED off by making the voltage LOW
   delay(1000);              // wait for a second

  // CRAP this conflicts with the A button!
  // float measuredvbat = analogRead(VBATPIN);
  // measuredvbat *= 2;    // we divided by 2, so multiply back
  // measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  // measuredvbat /= 1024; // convert to voltage
  // Serial.print("VBat: " ); Serial.println(measuredvbat);

  Serial.print("Mem:"); Serial.println(FreeRam());

  // if ( !digitalRead(BUTTON_B) ) {
  //   Serial.println("jdb");
  // }

  bool buttonPressed = false;
//  if (! digitalRead(BUTTON_A)) {
//    Serial.println("A");
//    display.clearDisplay();
//    display.display();
//    display.setCursor(0,0);
//    display.print("A");
//    buttonPressed = true;
//  }
  if (! digitalRead(BUTTON_B)) {
    Serial.println("B");
    display.clearDisplay();
    display.display();
    display.setCursor(0,0);
    display.print("B");
    buttonPressed = true;
  }
  if (! digitalRead(BUTTON_C)) {
    Serial.println("C");
    display.clearDisplay();
    display.display();
    display.setCursor(0,0);
    display.print("C");
    buttonPressed = true;
  }

  if (!buttonPressed){
    Serial.println("No button pressed");
    display.clearDisplay();
    display.display();
    display.setCursor(0,0);
    display.print("No button pressed");
  }
  // display.drawLine(0,0,display.width(),10, 1);// x0,y0,x1,y1,color

  display.setCursor(0,10);
  display.print(responseCount);

  delay(10);
  yield();
  display.display();
}

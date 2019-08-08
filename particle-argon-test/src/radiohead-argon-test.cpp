/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "application.h"
#line 1 "/home/jamis/my_root/dev/for_fun/radio/LoRa-tests/particle-argon-test/src/radiohead-argon-test.ino"
/*
 * Project radiohead-argon-test
 * Description:
 * Author:
 * Date:
 */

#include <SPI.h> // needed for radio frequency module
#include <RH_RF95.h> // Radio include for all lora feather9x_RX

#include "oled-wing-adafruit.h"

void displayRunning();
void setup();
void loop();
#line 13 "/home/jamis/my_root/dev/for_fun/radio/LoRa-tests/particle-argon-test/src/radiohead-argon-test.ino"
SYSTEM_THREAD(ENABLED);

#define LEDPIN D7

// For LoRa Featherwing on Particle board:
#define RFM95_RST     D6   // "A" wired to "RST"
#define RFM95_CS      D5   // "B" wired to "CS"
//#define RFM95_INT     D4   // "C" wired to "IRQ" THIS COLLIDES WITH OLED SCREEN BUTTONS
#define RFM95_INT     A5   // "F" wired to "IRQ"

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 433.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

OledWingAdafruit display;

int16_t packetnum = 0;  // packet counter, we increment per xmission
int16_t responseCount = 0;

bool rfmState = true;
bool rfmFreqSet = true;

void displayRunning() {
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("LoRa Ready!");
  display.printlnf("Freq set to: %.3f", RF95_FREQ);
  display.printf("Responses: %i", responseCount);
  display.display();
}

// setup() runs once, when the device is first turned on.
void setup() {
  Serial.begin(9600);
  delay(100);
  Serial.println("Feather LoRa TX Test!");

  display.setup();
	display.clearDisplay();
	display.display();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setRotation(0); //0 normal. 1 opposite usb top

  display.setCursor(0,0);
  display.println("Feather LoRa TX Test!");
  display.display(); // actually display all of the above

  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);              // wait for a second
  digitalWrite(LEDPIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);              // wait for a second

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

    // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("LoRa radio init failed!");
    display.display(); // actually display all of the above
    rfmState = false;
  }

  if (rfmState) {
    Serial.println("LoRa radio init OK!");
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("LoRa radio init OK!");
    display.display(); // actually display all of the above

    // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
    if (!rf95.setFrequency(RF95_FREQ)) {
      Serial.println("setFrequency failed");
      display.clearDisplay();
      display.setCursor(0,0);
      display.println("setFrequency failed");
      display.display(); // actually display all of the above
      rfmFreqSet = false;
    }

    if (rfmFreqSet) {
      Serial.print("Freq set to: ");
      Serial.println(RF95_FREQ);
      
      // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

      // The default transmitter power is 13dBm, using PA_BOOST.
      // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
      // you can set transmitter powers from 5 to 23 dBm:
      rf95.setTxPower(23, false);
      Serial.println("LoRa Ready!");
    }
  }
  
}


void loop() {
  if (rfmState && rfmFreqSet) {
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

    // Now wait for a reply
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    Serial.println("Waiting for reply..."); delay(10);
    if (rf95.waitAvailableTimeout(100)) {
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

    // Show recent status:
    displayRunning();

    digitalWrite(LEDPIN, HIGH);
    delay(100);
    digitalWrite(LEDPIN, LOW);
    delay(100);

    yield();
  }
}



// Solder closed jumper on bottom!
#include <Adafruit_NeoPixel_ZeroDMA.h>
#include <SPI.h>
#include <RH_RF69.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_FeatherOLED.h>

#define PIXEL_PIN    19    // Digital IO pin connected to the NeoPixels.
#define VBATPIN A7
#define PIXEL_COUNT 16.
#define LED_PIN 13
#define CPU_HZ 48000000
#define TIMER_PRESCALER_DIV 1000

#define AMBIENT_0 0
#define AMBIENT_1 1
#define AMBIENT_2 2
#define ACTIVE_0 3
#define ACTIVE_1 4
#define ACTIVE_2 5
#define STARTLE 6

int sampleRate = 1000; //sample rate of the square wave in Hertz, how many times per second the TC5_Handler() function gets called per second basically

void tcStartCounter(); //starts the timer
void TC5_Handler();

bool isLEDOn = false;
bool playFlag = false;
bool gestFlag = false;
bool noteFlag = false;
uint16_t noteNum = 64;
uint16_t gNum = 255;
uint16_t gCnt = 0;
uint16_t noteCnt = 7;
uint16_t noteLength = 50;
uint16_t duration = 50;
uint16_t tempo = 8;
uint16_t volume = 127;
uint16_t ledcount = 0;

uint8_t stateNum = 0;
uint8_t lastState = 0;
uint32_t counter = 0;
/************ Radio Setup ***************/

// Change to 434.0 or other frequency, must match RX's freq!
#define RF69_FREQ 915.0

// Feather M0 w/Radio
  #define RFM69_CS      8
  #define RFM69_INT     3
  #define RFM69_RST     4
  #define LED           13

// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

int16_t packetnum = 0;  // packet counter, we increment per xmission

Adafruit_FeatherOLED oled = Adafruit_FeatherOLED();

// See http://www.vlsi.fi/fileadmin/datasheets/vs1053.pdf Pg 31
#define VS1053_BANK_DEFAULT 0x00
#define VS1053_BANK_DRUMS1 0x78
#define VS1053_BANK_DRUMS2 0x7F
#define VS1053_BANK_MELODY 0x79

// See http://www.vlsi.fi/fileadmin/datasheets/vs1053.pdf Pg 32 for more!
#define VS1053_GM1_OCARINA 80

#define MIDI_NOTE_ON  0x90
#define MIDI_NOTE_OFF 0x80
#define MIDI_CHAN_MSG 0xB0
#define MIDI_CHAN_BANK 0x00
#define MIDI_CHAN_VOLUME 0x07
#define MIDI_CHAN_PROGRAM 0xC0

#define VS1053_MIDI Serial1

/*
uint8_t voice[] = {7,9,10,12,15,47,105,99,113,114,115,116,118,124 };

// ambient 0
uint8_t ambient0[8][3*8*8+2] = {
   {1,8*8,
   127,64,50,  127,69,50,  127,76,50,  127,75,50,  
   127,76,50,  127,83,50,  127,83,50,  127,64,50,  

   127,68,50,  127,76,50,  66,75,50,  127,76,50,  
   127,85,50,  127,83,50,  127,83,50,  127,78,50,  

   127,81,50,  127,84,50,  127,81,50,  127,81,50,  
   127,76,50,  127,76,50,  127,72,50,  127,72,50,  

   127,69,50,  127,76,50,  127,69,50,  127,64,50,  
   127,59,50,  127,78,50,  127,66,50,  127,59,50,  

   127,76,50,  127,69,50,  127,76,50,  127,75,50,  
   127,76,50,  127,75,50,  127,73,50,  127,71,50,  

   127,71,50,  127,75,50,  127,75,50,  127,73,50,  
   127,75,50,  127,73,50,  127,75,50,  127,73,50,

   127,75,50,  127,76,50,  127,76,50,  127,76,50,  
   127,69,50,  127,76,50,  127,75,50,  127,76,50,

   127,75,50,  127,73,50,  127,71,50,  127,71,50,  
   127,71,50,  127,76,50,  127,80,50,  127,73,50   },
};

// ambient 1
uint8_t ambient1[1][122] = {
   {7,40,
   127,80,40,   127,78,40,    127,75,80,    127,75,40,  
   127,73,40,   127,75,80,    127,71,80,    127,73,40,
   127,75,40,   127,73,40,    127,71,40,    127,68,80,  
   0,0,40,      127,70,40,    127,71,80,    127,73,80,
   127,75,80,   127,78,80,    127,73,160,   0,0,80,  
   127,80,40,   127,78,40,    127,75,80,    127,75,40,
   127,73,40,   127,75,80,    127,71,80,    127,73,40,
   127,75,40,   127,73,40,    127,71,40,    127,68,80,
   0,0,40,      127,70,40,    127,71,80,    127,75,80,
   127,73,80,   127,68,80,    127,68,120,   0,0,120
   }
   };

// ambient 2
uint8_t ambient2[1][169] = {
   {7,53,
   127,76,50,   127,74,50,    127,72,150,   127,71,50,  
   127,72,150,  127,74,50,    127,76,50,    127,81,50,
   127,81,50,   127,79,50,    127,76,100,   127,74,50,  
   127,71,50,   127,71,50,    127,72,50,    127,72,50,
   127,69,50,   127,69,50,    127,71,50,    127,71,50,  
   127,72,50,   127,74,100,   127,74,50,    127,76,50,
   127,74,50,   127,71,150,   127,69,50,    127,67,50,
   127,69,50,   127,71,50,    127,72,100,   127,72,50,
   127,74,50,   127,76,50,    127,81,50,    127,81,50,
   127,79,50,   127,76,100,   127,74,50,    127,71,50,
   127,72,50,   127,69,50,    127,69,50,    127,72,50,
   127,74,50,   127,71,50,    127,71,50,    127,71,50,
   127,71,50,   127,69,50,    127,69,50,    127,67,50,
   127,69,100
   }
   };

// startle
uint8_t startle[8][26] = {
   {1,8,
   127,46,40,  127,66,40,  127,68,40,  127,69,40,  
   127,78,40,  127,79,40,  127,85,40,  127,87,40   },  
   {1,8,
   127,88,40,  127,89,40,  66,90,40,  127,91,40,  
   127,92,40,  127,93,40,  127,94,40,  127,95,40   },  
   {1,8,
   127,96,40,  127,82,40,  127,83,40,  127,71,40,  
   127,84,40,  127,70,40,  127,72,40,  127,74,40   },  
   {1,8,
   127,75,40,  127,73,40,  127,42,40,  127,76,40,  
   127,88,40,  127,77,40,  127,89,40,  127,95,40   },  
   {1,8,
   127,78,40,  127,90,40,  127,96,40,  127,79,40,  
   127,80,40,  127,91,40,  127,97,40,  127,92,40   },  
   {1,8,
   127,98,40,  127,93,40,  127,99,40,  127,100,40,  
   127,66,40,  127,78,40,  127,85,40,  127,89,40   },
   {1,8,
   127,90,40,  127,91,40,  127,67,40,  127,69,40,  
   127,80,40,  127,86,40,  127,87,40,  127,88,40   },
   {1,8,
   127,92,40,  127,94,40,  127,95,40,  127,46,40,  
   127,68,40,  127,82,40,  127,96,40,  127,72,40   },
};

// active 0
uint8_t active0[1][2+3*8*10] = {
   {26,80,
   127,75,30,  127,75,30,  127,75,30,  127,82,120,  
   127,82,60,  127,80,30,  127,82,60,  127,84,30,
   
   127,84,60,  127,75,60,  127,82,120, 127,82,60,  
   127,80,30,  127,82,94,  127,84,60,  127,80,60,

   127,79,30,  127,80,156, 127,79,60,  127,77,60,  
   127,75,94,  127,80,156, 127,73,60,  127,75,30,
   
   127,77,60,  127,80,60,  127,84,60,  127,85,30,  
   127,82,60,  127,75,30,  127,75,30,  127,75,30,  

   127,82,120, 127,82,60,  127,80,30,  127,82,60,  
   127,84,30,  127,84,60,  127,75,60,  127,82,120,  

   127,82,60,  127,80,30,  127,82,94,  127,84,60,  
   127,85,60,  127,84,30,  127,80,156, 127,77,60,

   127,79,60,  127,80,30,  127,82,60,  127,84,156,  
   127,73,60,  127,80,30,  127,80,94,  127,82,45,

   127,84,120, 127,82,30,  127,80,60,  127,80,400,  
   127,75,60,  127,77,120, 127,77,60,  127,79,30,
   
   127,80,94,  127,80,60,  127,82,60,  127,84,60,
   127,82,94,  127,80,94,  127,80,187, 127,84,30,
   
   127,87,30,  127,92,30,  127,84,94,  127,87,94,
   127,92,94,  0,0,30,     0,0,30,     0,0,30  },
};

// active 1
uint8_t active1[1][197] = {
   {1,65,
   127,69,30,   127,72,30,    127,74,60,   127,74,60,  
   127,74,30,   127,76,30,    127,77,60,   127,77,60,
   127,77,30,   127,79,30,    127,76,60,   127,76,60,  
   127,74,30,   127,72,30,    127,72,30,   127,64,60,
   0,0,30,      127,69,30,    127,72,30,   127,74,60,  
   127,74,60,   127,74,30,    127,76,30,   127,77,60,
   127,77,60,   127,77,30,    127,79,30,   127,76,60,
   127,76,60,   127,74,30,    127,72,30,   127,74,60,
   0,74,60,     127,69,30,    127,72,30,   127,74,60,
   127,74,60,   127,74,30,    127,77,30,   127,79,60,
   127,79,60,   127,79,30,    127,81,30,   127,82,60,
   127,82,60,   127,81,30,    127,79,30,   127,81,30,
   127,74,60,   0,0,30,       127,74,30,   127,76,30,
   127,77,60,   127,77,60,    127,79,60,   127,81,30,
   127,74,60,   0,0,30,       127,74,30,   127,77,30,
   127,76,60,   127,76,60,    127,74,30,   127,72,30,
   127,74,30
   }
};

// active 2
uint8_t active2[1][101] = {
   {1,33,
   127,60,240,    127,67,240,   127,68,120,   127,70,120,  
   127,72,120,    127,65,40,    127,72,40,    127,70,40,
   127,68,80,     127,67,40,    127,60,40,    127,62,40,  
   127,64,40,     127,64,120,   127,63,40,    127,70,40,
   127,68,40,     127,67,80,    127,65,40,    127,58,40,  
   127,60,40,     127,62,160,   127,84,40,    127,84,20,
   127,85,20,     127,84,20,    127,80,40,    127,79,20,
   127,77,20,     127,79,20,    127,77,20,    127,70,20,    
   127,72,80
   }
};
*/
Adafruit_NeoPixel_ZeroDMA strip(PIXEL_COUNT, PIXEL_PIN, NEO_GRBW);

void setup() {
  delay(100);
  pinMode(LED_PIN, OUTPUT);
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  oled.display();
  oled.clearDisplay();
  oled.init();
  oled.setBatteryVisible(true);
  float battery = getBatteryVoltage();

  // update the battery icon
  oled.setBattery(battery);
  oled.renderBattery();
  oled.display();

  strip.begin();
  strip.setBrightness(5);
  strip.show(); // Initialize all pixels to 'off'

  colorWipe(strip.Color(0, 0, 0), 5);    // Black/off
  
  Serial.begin(115200);
  delay(100);
//  while (!Serial) { delay(1); } // wait until serial console is open, remove if not tethered to computer

  Serial.println("Board test");
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);
  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);
  
  if (!rf69.init()) {
    Serial.println("RFM69 radio init failed");
    while (1);
  }
  Serial.println("RFM69 radio init OK!");
  
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  // No encryption
  if (!rf69.setFrequency(RF69_FREQ)) {
    Serial.println("setFrequency failed");
  }

  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  rf69.setTxPower(14, true);

  // The encryption key has to be the same as the one in the server
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);
  
  Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");

  VS1053_MIDI.begin(31250); // MIDI uses a 'strange baud rate' 
//  midiSetChannelBank(0, VS1053_BANK_MELODY);
//  midiSetChannelVolume(0, 127);

  //tcConfigure(sampleRate); //configure the timer to run at <sampleRate>Hertz
  //tcStartCounter(); //starts the timer
}

void loop() {
  // clear the current count
  oled.clearDisplay();

  // get the current voltage of the battery from
  // one of the platform specific functions below
  float battery = getBatteryVoltage();

  // update the battery icon
  oled.setBattery(battery);
  oled.renderBattery();

  oled.setCursor(20, 15);
  oled.print("State: ");
  switch(stateNum) {
    case AMBIENT_0: oled.println("Ambient0"); break;
    case AMBIENT_1: oled.println("Ambient1"); break;
    case AMBIENT_2: oled.println("Ambient2"); break;
    case ACTIVE_0:  oled.println("Active0"); break;
    case ACTIVE_1:  oled.println("Active1"); break;
    case ACTIVE_2:  oled.println("Active2"); break;
    case STARTLE: oled.println("Startle"); break;
  }
  oled.display(); 
  
 // handle the radio 
  if (rf69.waitAvailableTimeout(100)) {
    // Should be a message for us now   
    uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    
    if (! rf69.recv(buf, &len)) {
      Serial.println("Receive failed");
       return;
    }
    digitalWrite(LED, HIGH);
    rf69.printBuffer("Received: ", buf, len);
    buf[len] = 0;
    SendPacket();
    
    Serial.print("Got: "); Serial.println((char*)buf);
    Serial.print("RSSI: "); Serial.println(rf69.lastRssi(), DEC);

    oled.println((char*)buf);
    oled.print("RSSI: "); oled.print(rf69.lastRssi());
    oled.display(); 
    digitalWrite(LED, LOW);
  }
  //handle sound
  //if (!playFlag) {
    //set Gesture 0-15
    //if(++gNum >0){
      //gNum = 0;

  switch(stateNum) {
    case AMBIENT_0: ambient1(100); break;
    case AMBIENT_1: ambient2(100); break;
    case AMBIENT_2: ambient3(100); break;
    case ACTIVE_0: chase(); break;
    case ACTIVE_1: theaterChaseRainbow(5); break;
    case ACTIVE_2: active3(); break;
    case STARTLE: startle(0xFF0000, 100); break;
  }
  
  if (counter > 3) {
    counter=0;
      if (stateNum != STARTLE)  lastState = stateNum;
      switch(stateNum) {
        case AMBIENT_0: stateNum = STARTLE; break;
        case AMBIENT_1: stateNum = STARTLE; break;
        case AMBIENT_2: stateNum = STARTLE; break;
        case ACTIVE_0:  stateNum = AMBIENT_1;  break;
        case ACTIVE_1:  stateNum = AMBIENT_2;  break;
        case ACTIVE_2:  stateNum = AMBIENT_0; break;
        case STARTLE: if(lastState == AMBIENT_0) stateNum = ACTIVE_0;
                      if(lastState == AMBIENT_1) stateNum = ACTIVE_1;
                      if(lastState == AMBIENT_2) stateNum = ACTIVE_2;
                      break;
      }
/*
    // set voice 0-14
    switch(stateNum) {
      case AMBIENT_0: midiSetInstrument(0, ambient0[gNum][0]); Serial.println(ambient0[gNum][0], DEC); break;
      case AMBIENT_1: midiSetInstrument(0, ambient1[gNum][0]); Serial.println(ambient1[gNum][0], DEC); break;
      case AMBIENT_2: midiSetInstrument(0, ambient2[gNum][0]); Serial.println(ambient2[gNum][0], DEC); break;
      case ACTIVE_0:  midiSetInstrument(0, active0[gNum][0]); Serial.println(active0[gNum][0], DEC); break;
      case ACTIVE_1:  midiSetInstrument(0, active1[gNum][0]); Serial.println(active1[gNum][0], DEC); break;
      case ACTIVE_2:  midiSetInstrument(0, active2[gNum][0]); Serial.println(active2[gNum][0], DEC); break;
      case STARTLE: midiSetInstrument(0, startle[gNum][0]); Serial.println(startle[gNum][0], DEC); break;
    }
*/
    noInterrupts();
    // critical, time-sensitive code here
    playFlag = true;

    //set Tempo?? 10-9
    if(--tempo >0) tempo = 9;
    interrupts();
  }

  // other code here
  // rainbowCycle(1);
  counter++;
}

void SendPacket() {
  {
    char radiopacket[] = "and Hello back to you";

    Serial.print("Sending: "); Serial.println(radiopacket);
    rf69.send((uint8_t *)radiopacket, sizeof(radiopacket));
    rf69.waitPacketSent();
  }
}
/*
void midiSetInstrument(uint8_t chan, uint8_t inst) {
  if (chan > 15) return;
  inst --; // page 32 has instruments starting with 1 not 0 :(
  if (inst > 127) return;
  
  VS1053_MIDI.write(MIDI_CHAN_PROGRAM | chan);  
  delay(10);
  VS1053_MIDI.write(inst);
  delay(10);
}


void midiSetChannelVolume(uint8_t chan, uint8_t vol) {
  if (chan > 15) return;
  if (vol > 127) return;
  
  VS1053_MIDI.write(MIDI_CHAN_MSG | chan);
  VS1053_MIDI.write(MIDI_CHAN_VOLUME);
  VS1053_MIDI.write(vol);
}

void midiSetChannelBank(uint8_t chan, uint8_t bank) {
  if (chan > 15) return;
  if (bank > 127) return;
  
  VS1053_MIDI.write(MIDI_CHAN_MSG | chan);
  VS1053_MIDI.write((uint8_t)MIDI_CHAN_BANK);
  VS1053_MIDI.write(bank);
}

void midiNoteOn(uint8_t chan, uint8_t n, uint8_t vel) {
  if (chan > 15) return;
  if (n > 127) return;
  if (vel > 127) return;
  
  VS1053_MIDI.write(MIDI_NOTE_ON | chan);
  VS1053_MIDI.write(n);
  VS1053_MIDI.write(vel);
}

void midiNoteOff(uint8_t chan, uint8_t n, uint8_t vel) {
  if (chan > 15) return;
  if (n > 127) return;
  if (vel > 127) return;
  
  VS1053_MIDI.write(MIDI_NOTE_OFF | chan);
  VS1053_MIDI.write(n);
  VS1053_MIDI.write(vel);
}
*/
void startle(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
  delay(wait);
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, 0x0000FF);
  }
  strip.show();
  delay(50);
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
  int color = random(0, 0xFFFFFF);
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256; j++) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256 * 4; j++) { // n cycles of all colors on wheel
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

static void chase() {
  int color = random(0, 0xFFFFFF);
  for(uint16_t i=0; i<strip.numPixels()+4; i++) {
      
      strip.setPixelColor(i  , color); // Draw new pixel
      strip.setPixelColor(i-4, 0); // Erase pixel a few steps back
      strip.show();
      delay(25);
  }
}

static void active3(){
  int color = random(0, 0xFFFFFF);
  for(uint16_t i=0; i < 4; i++) {           
    strip.setPixelColor(0+i, color); // Draw new pixel
    strip.setPixelColor(4+i, color); // Draw new pixel
    strip.setPixelColor(8+i, color); // Draw new pixel
    strip.setPixelColor(12+i, color); // Draw new pixel
    strip.show();
    delay(100);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void ambient1(uint8_t wait) {
   int color = random(0, 0xFFFFFF);
   for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
    strip.show();
    delay(wait);
  }
  color = random(0, 0xFFFFFF);
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
    strip.show();
    delay(wait);
  }
}

void ambient2(uint8_t wait) {
  for (uint16_t i = 0; i < (strip.numPixels()/2); i++) {
    strip.setPixelColor(i, 0xFFFFFF);
    strip.setPixelColor(i + (strip.numPixels()/2) - 1, 0xFFFFFF);
    strip.show();
    delay(wait);
    strip.setPixelColor(i, 0x000000);
    strip.setPixelColor(i + (strip.numPixels()/2) - 1, 0x000000);
    strip.show();
    delay(wait);
  }
  for (uint16_t i = 0; i < (strip.numPixels()/2); i++) {
    strip.setPixelColor((strip.numPixels()/2) - i - 1, 0xFFFFFF);
    strip.setPixelColor(strip.numPixels()-i -1, 0xFFFFFF);
    strip.show();
    delay(wait);
    strip.setPixelColor((strip.numPixels()/2) - i - 1, 0x000000);
    strip.setPixelColor(strip.numPixels()-i - 1, 0x000000);
    strip.show();
    delay(wait);
  }
}

void ambient3(uint8_t wait) {
  for (uint16_t i = 0; i < (strip.numPixels()/2); i++) {
    strip.setPixelColor(i, 0xFFFFFF);
    strip.setPixelColor(strip.numPixels()-i -1, 0xFFFFFF);
    strip.show();
    delay(wait);
    strip.setPixelColor(i, 0x000000);
    strip.setPixelColor(strip.numPixels()-i - 1, 0x000000);
    strip.show();
    delay(wait);
  }
  for (uint16_t i = 0; i < (strip.numPixels()/2); i++) {
    strip.setPixelColor((strip.numPixels()/2) - i - 1, 0xFFFFFF);
    strip.setPixelColor(i + (strip.numPixels()/2), 0xFFFFFF);
    strip.show();
    delay(wait);
    strip.setPixelColor((strip.numPixels()/2) - i - 1, 0x000000);
    strip.setPixelColor(i + (strip.numPixels()/2), 0x000000);
    strip.show();
    delay(wait);
  }
}

float getBatteryVoltage() {

  float measuredvbat = analogRead(VBATPIN);

  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage

  return measuredvbat;

}
/*
//Configures the TC to generate output events at the sample frequency.
//Configures the TC in Frequency Generation mode, with an event output once
//each time the audio sample frequency period expires.

void tcConfigure(int sampleRate)
{
 // Enable GCLK for TCC2 and TC5 (timer counter input clock)
 GCLK->CLKCTRL.reg = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TC4_TC5)) ;
 while (GCLK->STATUS.bit.SYNCBUSY);

 tcReset(); //reset TC5

 // Set Timer counter Mode to 16 bits
 TC5->COUNT16.CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
 // Set TC5 mode as match frequency
 TC5->COUNT16.CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
 //set prescaler and enable TC5
 TC5->COUNT16.CTRLA.reg |= TIMER_PRESCALER_DIV | TC_CTRLA_ENABLE;
 //set TC5 timer counter based off of the system clock and the user defined sample rate of waveform
 TC5->COUNT16.CC[0].reg = (uint16_t) (CPU_HZ / sampleRate - 1);
 while (tcIsSyncing());
 
 // Configure interrupt request
 NVIC_DisableIRQ(TC5_IRQn);
 NVIC_ClearPendingIRQ(TC5_IRQn);
 NVIC_SetPriority(TC5_IRQn, 0);
 NVIC_EnableIRQ(TC5_IRQn);

 // Enable the TC5 interrupt request
 TC5->COUNT16.INTENSET.bit.MC0 = 1;
 while (tcIsSyncing()); //wait until TC5 is done syncing 
} 

//Function that is used to check if TC5 is done syncing
//returns true when it is done syncing
bool tcIsSyncing()
{
  return TC5->COUNT16.STATUS.reg & TC_STATUS_SYNCBUSY;
}

//This function enables TC5 and waits for it to be ready
void tcStartCounter()
{
  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE; //set the CTRLA register
  while (tcIsSyncing()); //wait until snyc'd
}

//Reset TC5 
void tcReset()
{
  TC5->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
  while (tcIsSyncing());
  while (TC5->COUNT16.CTRLA.bit.SWRST);
}

//disable TC5
void tcDisable()
{
  TC5->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (tcIsSyncing());
}


//this function gets called by the interrupt at <sampleRate>Hertz
void TC5_Handler (void) {
  switch(stateNum) {
    case AMBIENT_0:
      if(playFlag){
        if(!gestFlag){
          // start Gesture
          gestFlag = true;
          //set Voice
          gCnt = 1;
          //set notecnt
          noteCnt = ambient0[gNum][gCnt++];
        }
        if(!noteFlag){
          //set volume
          volume = ambient0[gNum][gCnt++];
          //set noteNum
          noteNum = ambient0[gNum][gCnt++];
          //set duration
          duration = ambient0[gNum][gCnt++]-tempo;
  
         //start note
          midiNoteOn(0, noteNum, volume);
          noteFlag = true;
          digitalWrite(LED_PIN, isLEDOn);
          isLEDOn = !isLEDOn;
       }
        duration -= 1;
        if(duration==0){
          noteCnt -= 1;
          if(noteCnt==0){
            //clear all flags
            playFlag = false;
            gestFlag = false;
          }
          noteFlag = false;
        }
      }break;
    case AMBIENT_1:
      if(playFlag){
          if(!gestFlag){
            // start Gesture
            gestFlag = true;
            //set Voice
            gCnt = 1;
            //set notecnt
            noteCnt = ambient1[gNum][gCnt++];
            
          }
          if(!noteFlag){
            //set volume
            volume = ambient1[gNum][gCnt++];
            //set noteNum
            noteNum = ambient1[gNum][gCnt++];
            //set duration
            duration = ambient1[gNum][gCnt++]-tempo;
    
           //start note
            midiNoteOn(0, noteNum, volume);
            noteFlag = true;
            digitalWrite(LED_PIN, isLEDOn);
            isLEDOn = !isLEDOn;
         }
          duration -= 1;
          if(duration==0){
            noteCnt -= 1;
            if(noteCnt==0){
              //clear all flags
              playFlag = false;
              gestFlag = false;
            }
            noteFlag = false;
          }
        }break;
    case AMBIENT_2:
      if(playFlag){
        if(!gestFlag){
          // start Gesture
          gestFlag = true;
          //set Voice
          gCnt = 1;
          //set notecnt
          noteCnt = ambient2[gNum][gCnt++];
          
        }
        if(!noteFlag){
          //set volume
          volume = ambient2[gNum][gCnt++];
          //set noteNum
          noteNum = ambient2[gNum][gCnt++];
          //set duration
          duration = ambient2[gNum][gCnt++]-tempo;
  
         //start note
          midiNoteOn(0, noteNum, volume);
          noteFlag = true;
          digitalWrite(LED_PIN, isLEDOn);
          isLEDOn = !isLEDOn;
       }
      duration -= 1;
      if(duration==0){
        noteCnt -= 1;
        if(noteCnt==0){
          //clear all flags
          playFlag = false;
          gestFlag = false;
        }
        noteFlag = false;
      }
    }break;
    case ACTIVE_0:
      if(playFlag){
        if(!gestFlag){
          // start Gesture
          gestFlag = true;
          //set Voice
          gCnt = 1;
          //set notecnt
          noteCnt = active0[gNum][gCnt++];
          
        }
        if(!noteFlag){
          //set volume
          volume = active0[gNum][gCnt++];
          //set noteNum
          noteNum = active0[gNum][gCnt++];
          //set duration
          duration = active0[gNum][gCnt++]-tempo;
  
         //start note
          midiNoteOn(0, noteNum, volume);
          noteFlag = true;
          digitalWrite(LED_PIN, isLEDOn);
          isLEDOn = !isLEDOn;
       }
      duration -= 1;
      if(duration==0){
        noteCnt -= 1;
        if(noteCnt==0){
          //clear all flags
          playFlag = false;
          gestFlag = false;
        }
        noteFlag = false;
      }
    }break;
    case ACTIVE_1:
      if(playFlag){
        if(!gestFlag){
          // start Gesture
          gestFlag = true;
          //set Voice
          gCnt = 1;
          //set notecnt
          noteCnt = active1[gNum][gCnt++];
          
        }
        if(!noteFlag){
          //set volume
          volume = active1[gNum][gCnt++];
          //set noteNum
          noteNum = active1[gNum][gCnt++];
          //set duration
          duration = active1[gNum][gCnt++]-tempo;
  
         //start note
          midiNoteOn(0, noteNum, volume);
          noteFlag = true;
          digitalWrite(LED_PIN, isLEDOn);
          isLEDOn = !isLEDOn;
       }
      duration -= 1;
      if(duration==0){
        noteCnt -= 1;
        if(noteCnt==0){
          //clear all flags
          playFlag = false;
          gestFlag = false;
        }
        noteFlag = false;
      }
    }break;
    case ACTIVE_2:
      if(playFlag){
        if(!gestFlag){
          // start Gesture
          gestFlag = true;
          //set Voice
          gCnt = 1;
          //set notecnt
          noteCnt = active2[gNum][gCnt++];
          
        }
        if(!noteFlag){
          //set volume
          volume = active2[gNum][gCnt++];
          //set noteNum
          noteNum = active2[gNum][gCnt++];
          //set duration
          duration = active2[gNum][gCnt++]-tempo;
  
         //start note
          midiNoteOn(0, noteNum, volume);
          noteFlag = true;
          digitalWrite(LED_PIN, isLEDOn);
          isLEDOn = !isLEDOn;
       }
      duration -= 1;
      if(duration==0){
        noteCnt -= 1;
        if(noteCnt==0){
          //clear all flags
          playFlag = false;
          gestFlag = false;
        }
        noteFlag = false;
      }
    }break;
    case STARTLE:
      if(playFlag){
        if(!gestFlag){
          // start Gesture
          gestFlag = true;
          //set Voice
          gCnt = 1;
          //set notecnt
          noteCnt = startle[gNum][gCnt++];
          
        }
        if(!noteFlag){
          //set volume
          volume = startle[gNum][gCnt++];
          //set noteNum
          noteNum = startle[gNum][gCnt++];
          //set duration
          duration = startle[gNum][gCnt++]-tempo;
  
         //start note
          midiNoteOn(0, noteNum, volume);
          noteFlag = true;
          digitalWrite(LED_PIN, isLEDOn);
          isLEDOn = !isLEDOn;
       }
      duration -= 1;
      if(duration==0){
        noteCnt -= 1;
        if(noteCnt==0){
          //clear all flags
          playFlag = false;
          gestFlag = false;
        }
        noteFlag = false;
      }
    }break;
  }
  TC5->COUNT16.INTFLAG.bit.MC0 = 1; //don't change this, it's part of the timer code
}
*/

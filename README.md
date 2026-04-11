# Use-RC522-to-Toggle-Fadding-and-Blinking-LED-via-SPI

This project uses ESP32 with RC522 RFID module to toggle LED modes (fading and blinking) based on scanned RFID cards.

## Features
- Read RFID tag and card using RC522
- Toggle LED
- LED fading using PWM (LEDC)
- LED blinking mode

## Hardware Required
- ESP32
- RCC522 RFID Module
- 2 x LED
- 2 x Resistor 100 ~ 300 Ω
- Jumper wire

## Wiring
1. RC522   => ESP32
2. SDA     => GPIO5    
3. SCK     => GPIO18
4. MOSI    => GPIO23
5. MISO    => GPIO19
6. RST     => GPIO22
7. GND     => GND
8. 3.3V    => 3.3V

9. ESP32.GPIO15 => Resistor => LED => GND
10. ESP32.GPIO16 => Resistor => LED => GND

## Setup
1. Install ESP-IDF and git
2. Clone this repository:
    git clone https://github.com/AlgoRhythm23225/Use-RC522-to-Toggle-Fadding-and-Blinking-LED.git
3. Build Project 
    Open IDF_v6.0_Powershell
    cd to your project's forlder
    idf.py build
4. Flash to ESP32
    Open Devide Manager to check which COM do your ESP32 is using
    In my case:
         idf.py -p COM9 flash monitor

## How It Works
- System waits for RFID card/tag
- First, take 2 card or tag for on and off
- Let RC522 read you card and print result to terminal
- OPEN main/rc522_esp.c
    Change this marco to the first 2 character of card/tag ID
        #define TAG_ID  0x51    // ON    
        #define CARD_ID 0x2F    // OFF  
- How it work: 
    In the first flash (run): LEDS are off
    Push tag close to RC522, LEDS start to fadding and Blinking
    Push card close, LEDS off again

    

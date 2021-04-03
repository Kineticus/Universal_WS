#  Chill WS2811 Pixel Control

Arduino ATmega328 based. Started life as a Burning Man 2011 project. Controls up to 150 pixels. 

## Install

This project requires the [Arduino IDE](https://www.arduino.cc/en/Main/Software).

1. Install [Arduino IDE](https://www.arduino.cc/en/Main/Software)
2. Add the "Adafruit NeoPixel" library (Tools > Manage Libraries...)


## Wiring
 
1. Brightness Potentiometer
   - A0   Ground
   - A1   5v
   - A2   Wiper
  
2. Speed Potentiometer
   - A3   Ground
   - A4   5v
   - A5   Wiper
  
3. Rotary Encoder
   - D2   A Channel
   - D3   B Channel
   - D4   Button (normally floating, grounded on click)
   - D5   Ground
  
4. LEDs
   - D6   WS2811 Data Out

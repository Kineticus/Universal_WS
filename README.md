#  Chill WS2811 Pixel Control

Arduino ATmega328 based. Started life as a Burning Man 2013 project. Controls up to 150 pixels in a relaxed manner. 

## Install

This project requires the [Arduino IDE](https://www.arduino.cc/en/Main/Software).

1. Install [Arduino IDE](https://www.arduino.cc/en/Main/Software)
2. Add the "Adafruit NeoPixel" library (Tools > Manage Libraries...)


## Wiring
 
1. Brightness Potentiometer
   - A0   Ground
   - A1   Vcc (5v or less)
   - A2   Wiper
  
2. Speed Potentiometer
   - A3   Ground
   - A4   Vcc (5v or less)
   - A5   Wiper
  
3. Rotary Encoder
   - D2   A Channel
   - D3   B Channel
   - D4   Button (normally floating, grounded on click)
   - D5   Ground
  
4. LEDs
   - D6   WS2811 Data Out


## Modes

1.	Amber
2.	Warm White
3.	Neutral White
4.	Bright White
5.	Cool White
6.	Red > Yellow
7.	Yellow > Green
8.	Green > Teal
9.	Teal > Blue
10.	Blue > Purple
11.	Purple > Red
12.	Blue & Amber
13.	Blue & Green
14.	Purple & Gold
15.	White & Amber
16.	Green & Red
17.	Pink & Blue
18.	Blue & White
19.	Red & White
20.	Green & White
21.	Green & Gold
22.	Teal & Bronze
23.	Solid Rainbow
24.	Half Rainbow
25.	Full Rainbow
26.	Triple Color
27.	Two Color Wide (First Complement)
28.	Two Color Wide (Second Complement)
29.	Two Color Wide (Third Complement)
30.	Color Wipe Bounce
31.	Color Wipe
32.	Valentine's Day
33.	St. Patrick's Day
34.	Easter
35.	April 20th
36.	Father's Day
37.	4th of July
38.	Maryland
39.	Halloween
40.	Authumn
41. Christmas
42.	Christmas Lights
43.	Tibetan Flag
44.	Glowing Amber
45.	Flow - Y
46.	Flow - Y - Orange Glow
47.	Flow - X
48.	Flow - X - Orange Glow

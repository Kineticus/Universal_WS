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
21.	Christmas Lights
22.	Half Rainbow
23.	Full Rainbow
24.	Solid Rainbow
25.	Two Color Wide 1 (Small Spacing)
26.	Two Color Wide 2 (First Complement)
27.	Two Color Wide 3 (Second Complement)
28.	Two Color Wide 4 (Third Complement)
29.	Three Color
30.	Color Wipe
31.	Water Flow
32.	Color Wipe Bounce
33.	Sparkle Rainbow
34.	Sparkle Purple & Gold
35.	Sparkle Purple & Green
36.	Sparkle Rasta
37.	Sparkle Amber & White
38.	Sparkle Amber
39.	Flow - Y - Small - None
40.	Flow - Y - Small - Every 4
41.	Flow - Y - Small - Every 7
42.	Flow - X - Small - None
43.	Flow - X - Small - Every 4
44.	Flow - X - Small - Every 7
45.	Flow - Y - Medium - None
46.	Flow - Y - Medium - Every 4
47.	Flow - Y - Medium - Every 7
48.	Flow - X - Medium - None
49.	Flow - X - Medium - Every 4
50.	Flow - X - Medium - Every 7
51.	Flow - Y - Large - None
52.	Flow - Y - Large - Every 4
53.	Flow - Y - Large - Every 7
54.	Flow - X - Large - None
55.	Flow - X - Large - Every 4
56.	Flow - X - Large - Every 7

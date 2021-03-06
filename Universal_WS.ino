/***************************************************************************************
****************************************************************************************

Project: Large Amounts of WS281X Lights for Anything!

Description: Controls a WS281X based strands of LEDs. Accepts input from a Rotary
             Encoder (pattern select) and Potentiometer (brightness). Stores current 
             pattern in to memory for recall after power loss.

    
    
Change log: 

Version 2.0	- Brian	- Added interfade / smoothOperator from Gemina with scaling
	2/10/2021			Dynamic color with 2nd knob and better pattern organization
						Metal Baseline

Version 1.9 - Brian - Reduced number of total functions, simplified structure, small fixes
	1/23/2021			This was sent to Bryson and Lauren

Version 1.8 - Brian - Added more patterns and "AmberSmatter" options to color flow
	12/16/2020			This will be sent to James and Susan
						
Version 1.7 - Brian - Added robust speed knob support

Version 1.6
	1/19/20 - Brian - Fixed glitching on Simplex Noise patterns (maybe)
						Backwards compatability for 2018 boxes

Version 1.5
	6/15/19 - Brian - Smoothing of things :)
						Fixed several small glitches
						More fade control patterns 
						This was sent to Rob McAllister

Version 1.4
	6/13/19 - Matt/Brian - Refined programs, added robust speed knob support

Version 1.3 
	6/10/19 - Brian - Support for 150 pixels with Scaling

Version 1.2
	2/4/19 - Brian - Converted back to Rotary encoder/knobs
									 more color programs
									configurable SPEED knob (see config)
									STRAND TYPE IS BELOW CONFIG! GRB OR RGB

Version 1.1
  9/1/18 - Matt/Brian - More better functions

Version 1.0
  9/1/18 - Brian - Changed to DMX
				Patched overflow on Simplex Noise function (xoffset/yoffset)

Version 0.9
  12/10/16 - Matt/Brian - Created baseline code to interface with rotary encoder
                          and potentiometer. Ported simplex noise code and created
                          basic color changers and hues.
                          
Version 0.9.1                        
  12/13/16 - Matt       - Added more colors and refined rainbow/noise flows.

Version 0.9.2
  12/15/16 - Brian      - Optimized output code for higher frame rate    

Future Improvements:

  
****************************************************************************************
***************************************************************************************/

/***************************************************************************************
  Libraries
***************************************************************************************/
#include <Adafruit_NeoPixel.h>	//WS type strips
#include <EEPROM.h>        		//Persistent Memory


/***************************************************************************************
  !!!! >>>>> LOOK HERE <<<<<< !!!! ------ P O T    W I R I N G    S E T U P

	Define your Pots! Here is the "standard"

	Brightness Potentiometer:		(BLUE/GREEN/yellow/ORANGE)
		/-> GND (alternative A0)	(BLUE)
		Q-> A1						(GREEN)
		\-> VCC (alternative A3)	(ORANGE)
	
  	Speed Potentiometer:			(BROWN/RED/ORANGE/yellow)
    	/-> GND (alternative A4)	(BROWN)
    	Q-> A6						(ORANGE)
    	\-> VCC (alternative A5)	(RED)


	The Encoder pins are NOT adjustable:

	Rotary Encoder:				(BLACK/WHITE/GREY/PURPLE)
    D 3 <-   -> D 4 			(D3 GREY) 	(D4 PURPLE)
    GND <- O  |					(BLACK)
    D 2 <-   -> GND				(D2 WHITE)	(BLACK)

***************************************************************************************/

#define PIN_BRIGHT 		A2		//Brightness Potentiometer Wiper
#define INVERT_BRIGHT 	0		//Brightness direction
#define PIN_POWER_A		A1		//AUX 5v
#define PIN_GROUND_A	A0		//AUX Ground
#define SPEED_KNOB 		1 		//Enable 2nd Potentiometer for Speed (0 = OFF, 1 = ON)
#define PIN_SPEED		A5		//
#define INVERT_SPEED 	0		//Speed direction
#define PIN_POWER_B		A3		//AUX 5v
#define PIN_GROUND_B	A4		//AUX Ground

#define SENSITIVITY		5
#define BRIGHT_FLOOR	2

/***************************************************************************************
  !!!! >>>>> LOOK HERE <<<<<< !!!! ----- P I X E L   W I R I N G    S E T U P

	Parameter 1 = number of pixels in strip
	Parameter 2 = Arduino pin number (most are valid)
	Parameter 3 = pixel type flags, add together as needed:
	NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
	NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
	NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
	NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
	NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

	// Example for RGB Ping Pong Balls:
	//Adafruit_NeoPixel strip = Adafruit_NeoPixel(maxPixels, PIN_LED, NEO_RGB + NEO_KHZ800);

	// Example for RGB Strip/Reel:
	Adafruit_NeoPixel strip = Adafruit_NeoPixel(maxPixels, PIN_LED, NEO_GRB + NEO_KHZ800);

***************************************************************************************/

#define maxPixels 		100		//Number of Pixels (add 1 extra for each level of upscaling)
#define UPSAMPLE 		2  		//1 is No upsampling, up to maximum of 3
#define interfadeMax	8		//Fade time between patterns in frames
#define PIN_DATA 		6		//WS28XX data line

Adafruit_NeoPixel strip = Adafruit_NeoPixel(maxPixels, PIN_DATA, NEO_RGB + NEO_KHZ800);

byte ledTemp[(maxPixels / UPSAMPLE)][3];

/***************************************************************************************
 						>>>>>> END OF CONFIGURATION <<<<<<
***************************************************************************************/



/***************************************************************************************
  General Variables
***************************************************************************************/
int currBrightness = 0;
int tempValue = 0;
int brightnessValue = 0;
int speedValue = 1023;
int currSpeed = 2;
int currProgram = 2;
int currFade = 0;
int currFadeStep = 0;
int currStep = 0;
int tempStepEffect = 0;
long brightnessTotal = 0;
int brightnessCount = 0;
long speedTotal = 0;
int speedCount = 0;
int writeDelay = 0;
byte interfade = 0;
byte tempStepDirection = 0;

int fadeDirection = 0; // 1 or 0, positive or negative
int fadeDirection2 = 0; // 1 or 0, positive or negative
int fadeDirectionHTemp = .5; // 1 or 0, positive or negative

unsigned long fpsMillis = 0;
unsigned long effectMillis = 0;
unsigned long currMillis = 0;
int brightnessRead = 0;


/***************************************************************************************
  Encoder Variables
***************************************************************************************/

byte upperLimit = 54; // max number of patterns (encoder)
int lowerLimit = 1; // minimum number of patterns (encoder)
int encoderButton = 49;
byte lastSavedEncoderPosition = 0;
int counter = 0;

static int pinA = 2; // Our first hardware interrupt pin is digital pin 2
static int pinB = 3; // Our second hardware interrupt pin is digital pin 3
volatile byte aFlag = 0; // let's us know when we're expecting a rising edge on pinA to signal that the encoder has arrived at a detent
volatile byte bFlag = 0; // let's us know when we're expecting a rising edge on pinB to signal that the encoder has arrived at a detent (opposite direction to when aFlag is set)
volatile byte encoderPos = 0; //this variable stores our current value of encoder position. Change to int or uin16_t instead of byte if you want to record a larger range than 0-255
volatile byte oldEncPos = 0; //stores the last encoder position value so we can compare to the current reading and see if it has changed (so we know when to print to the serial monitor)
volatile byte reading = 0; //somewhere to store the direct values we read from our interrupt pins before checking to see if we have moved a whole detent


/***********************************************************
  Simplex Noise Variable Declaration
***********************************************************/
//Define simplex noise node for each LED
const int LEDs_in_strip = (maxPixels / UPSAMPLE)+3;
const int LEDs_for_simplex = 6;

// Extra fake LED at the end, to avoid fencepost problem.
// It is used by simplex node and interpolation code.
float LED_array_red[LEDs_in_strip+1];
float LED_array_green[LEDs_in_strip+1];
float LED_array_blue[LEDs_in_strip+1];
int node_spacing = LEDs_in_strip / LEDs_for_simplex;

// Math variables
int i, j, k, A[] = {0, 0, 0};
float x1,y1,x2,y2;
float u, v, w, s, h;
float hTemp = .420;
float hOld = .0;
static float onethird = 0.333333333;
static float onesixth = 0.166666667;
int T[] = {0x15, 0x38, 0x32, 0x2c, 0x0d, 0x13, 0x07, 0x2a};

// Simplex noise parameters:
float timeinc = 0.0025;
float spaceinc = 0.05;
int intensity_r = 734;
int intensity_g = 734;
int intensity_b = 734;
float yoffset = 0.0;
float xoffset = 0.0;
float saturation = 1.0;
int red, green, blue;
int red2, green2, blue2;  //used in HSV color functions 
int red3, green3, blue3;
int tempStep = 0;


/***************************************************************************************
  Setup
***************************************************************************************/
void setup()
{
  /***************************************************************************************
    General Setup
  ***************************************************************************************/
 	Serial.begin(115200);

	//Randomize the Simplex Noise values for lava lamp style patterns
	//Create a random seed by reading nearby electric noise on the analog ports
	randomSeed(analogRead(A6) + analogRead(A2) + analogRead(A3) + analogRead(A4) + analogRead (A5));
	
 	//Randomish x and y offset for simplex noise
 	yoffset = analogRead(A4) + analogRead (A5);
 	xoffset = analogRead(A5) + analogRead(A3);

	//Seed color
	h = random(500) / 1000.0;
    hOld = h + random(500) / 1000.0;
	
	//Set the pins 
	pinMode(PIN_POWER_A, OUTPUT);
	digitalWrite(PIN_POWER_A, HIGH);
	
	pinMode(PIN_GROUND_A, OUTPUT); 
	digitalWrite(PIN_GROUND_A, LOW);

	pinMode(PIN_BRIGHT, INPUT); 

	pinMode(PIN_POWER_B, OUTPUT);
	digitalWrite(PIN_POWER_B, HIGH); 

	pinMode(PIN_GROUND_B, OUTPUT);
	digitalWrite(PIN_GROUND_B, LOW);

	pinMode(PIN_SPEED, INPUT); 

  /***************************************************************************************
    Encoder Setup
  ***************************************************************************************/
  	//Set the pin for the Encoder's push down button
	pinMode(4, INPUT);		//Enable reading of PIN state (ON or OFF)
	digitalWrite(4, HIGH); 	//Enables internal Pull Up resistor which creates a bias towards 5v (ON)
	
	//Set Ground for Pin 5, backwards compatability
	pinMode(5, OUTPUT);
	digitalWrite(5, LOW);

	//Set the pins for the Encoder's Data Interrupt Pins
	//NOTE: See "Encoder Variables" configuration above for definitions and settings
	pinMode(pinA, INPUT_PULLUP); // set pinA as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
	pinMode(pinB, INPUT_PULLUP); // set pinB as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
	attachInterrupt(0,PinA,RISING); // set an interrupt on PinA, looking for a rising edge signal and executing the "PinA" Interrupt Service Routine (below)
	attachInterrupt(1,PinB,RISING); // set an interrupt on PinB, looking for a rising edge signal and executing the "PinB" Interrupt Service Routine (below)

 /***************************************************************************************
    Other Setup
  ***************************************************************************************/
	//Initialize up the LED strip
	strip.begin();
	
	//Display one frame to the the strip, to start they are all 'off'
	strip.show();

	//Read from the onboard persistant memory for last program position
	lastSavedEncoderPosition = EEPROM.read(1);
	
	if (lastSavedEncoderPosition > upperLimit)
	{
		lastSavedEncoderPosition = 1;
	}
	
	//Set program to value from memory (a new board has 0 in EEPROM for each slot)
	encoderPos = lastSavedEncoderPosition;
	oldEncPos = lastSavedEncoderPosition;

	//Read the inputs once for prep
	readInputs();

	//Begin interfade for smooth fade in
	smoothFadeBegin();
}

/***************************************************************************************
  Main Loop
***************************************************************************************/
void loop()
{

	//Read potentiometer values
	readInputs();

	//Framerate Control
	if (fpsMillis < millis())
  	{
		//1 second = 1000 millis. 1000 millis / 60 fps = 16 millis between frames
		fpsMillis = millis() + 16;

		//Check to see if we should write the current encoder value to EEPROM
		if (writeDelay > 1)
		{
			writeDelay --;
		}
		else if (writeDelay == 1)
		{
			EEPROM.write(1, encoderPos);
			writeDelay = 0;
		}

		//Calculate average potentiometer readings
		smoothOperator();

		//Check to see if Encoder value has changed
		if (encoderPos != oldEncPos)
		{
			//Keep encoder knob within upper && lower limits
			if (encoderPos > upperLimit){
				encoderPos = lowerLimit;
			}
			if (encoderPos < lowerLimit) {
				encoderPos = upperLimit;
			}

			//Start new interfade
			smoothFadeBegin();
			
			//Start countdown to store encoder setting to EEPROM
			writeDelay = 252;
			
			//Update the oldEncPos value to current
			oldEncPos = encoderPos;
		}

		// display appropriate pattern
		callColorFunction();

		if (interfade != 0)
		{
			//smoothFade accepts 0 to 255, 255 is full snapshot, 0 is full current
			smoothFade(interfade * (255 / interfadeMax));
		}

		if (interfade > 0)
		{
			interfade --;
		}
		
		//Transmit one frame of data out
		strip.show();
	}
}

void callColorFunction()
{
	// select color
  	switch (encoderPos-1) {
		
		//Single Colors
		case 0:
			//Red > Amber
			singleColor(currBrightness,currBrightness * (float(currSpeed) / 420),0);
			break;
		case 1:  
			//Amber > Warm White
			singleColor(currBrightness, (currBrightness/5) + (currBrightness * (float(currSpeed) / 420)), (currBrightness/30) + (currBrightness * (float(currSpeed) / 1400)));
			break;
		case 2:
			//Warm White > Neutral White
			singleColor(currBrightness, (currBrightness/3) + (currBrightness * (float(currSpeed) / 400)), (currBrightness/12) + (currBrightness * (float(currSpeed) / 1200)));
			//singleColor(currBrightness, currBrightness/4, currBrightness/25);
			break;
		case 3:
			//Neutral White > Day Light
			singleColor(currBrightness, (currBrightness/2.5) + (currBrightness * (float(currSpeed) / 300)), (currBrightness/8) + (currBrightness * (float(currSpeed) / 350)));
			//singleColor(currBrightness, currBrightness/3.5,currBrightness/15);
			break;
		case 4:
			//Day Light > Cool Light
			singleColor(currBrightness, (currBrightness/2) + (currBrightness * (float(currSpeed) / 300)), (currBrightness/4) + (currBrightness * (float(currSpeed) / 150)));
			//singleColor(currBrightness, currBrightness/3,currBrightness/10);
			break;
		case 5:
			//Red > Yellow
			singleColor(currBrightness,currBrightness * (float(currSpeed) / 100),0);
			break;
		case 6:
			//Yellow > Green
			singleColor(currBrightness * (float(100 - currSpeed) / 100),currBrightness,0);
			break;
		case 7:
			//Green > Teal
			singleColor(0,currBrightness,currBrightness * (float(currSpeed) / 100));
			break;
		case 8:   
			//Teal > Blue
			singleColor(0,currBrightness * (float(100 - currSpeed) / 100),currBrightness);        
			break;
		case 9:
			//Blue > Purple
			singleColor(currBrightness * (float( currSpeed) / 100),0,currBrightness);
			break;
		case 10:
			//Purple > Red
			singleColor(currBrightness,0,currBrightness * (float(100 - currSpeed) / 100));
			//rainbowFlag();
			break;
		// --- Two Colors ---
		case 11:
			//Blue and Amber
			SteadyAlternatingColors(0,0,currBrightness, currBrightness, currBrightness/5, 0, (currSpeed + 1) / 6);
			//SparkleBlueGreen();
			break;
		case 12:
			//Blue and Green
			SteadyAlternatingColors(0,0,currBrightness, 0, currBrightness, 0, (currSpeed + 1) / 6);break;
		case 13:
			//Purple and gold
			SteadyAlternatingColors(currBrightness,0,currBrightness/1.5, currBrightness, currBrightness/3, 0, (currSpeed + 1) / 6);
			break;
		case 14:
			//White and Amber
			SteadyAlternatingColors(currBrightness/2,currBrightness/2,currBrightness/3, currBrightness, currBrightness/4, 0, (currSpeed + 1) / 6);
		break;
		case 15:
			//Green and Red
			SteadyAlternatingColors(0,currBrightness,0, currBrightness, 0, 0, (currSpeed + 1) / 6);
			break;
		case 16:
			//Pink and Blue
			SteadyAlternatingColors(currBrightness,0,currBrightness/2, 0, 0, currBrightness, (currSpeed + 1) / 6);
			break;
		case 17:
			//Blue and White
			SteadyAlternatingColors(0,0,currBrightness, currBrightness/2,currBrightness/2,currBrightness/2, (currSpeed + 1) / 6);
			break;
		// --- Rainbow Type Things ---
		case 18:
			christmasLights();
			effectFunction();
			break;
		case 19:
			//1/2 Rainbow
			RainbowFlow(float(0.5 / maxPixels));
			break;
		case 20:
			//Full Rainbow
			RainbowFlow(float(1.0 / maxPixels));
			break;
		case 21:
			//Whole strip go through single HSV colors
			RainbowHsv();
			break;
		case 22:   
			DualColorFlow();
			break;
		case 23:   
			DualColorFlowFat();
			break;
		case 24:
			DualColorFlowFat2();
			break;
		case 25:
			DualColorFlowFat3();
			break;
		case 26:
			triFlag();
			break;
		case 27:
			colorWipe();
			//DualColorFlowGreenFast();
			break;
		case 28:
			waterFlow();
			break;
		case 29:
			colorWipeBounce();
			break;
		case 30:
			TwinkleRainbow();
			break;
		case 31:
			PurpleGoldSparkle();
			break;
		case 32:
			PurpleGreen();
			break;
		case 33:
			sparkleRasta();
			break;
		case 34:
			GlowingAmberWhite();
			break;
		case 35:
			GlowingAmber();
			break;
		case 36:
			RainbowYoffset(.03);
			break;
		case 37:
			RainbowYoffset(.03);
			AmberSmatter(4);
			break;
		case 38:
			RainbowYoffset(.03);
			AmberSmatter(7);
			break;
		case 39:
			RainbowXoffset(.03);
  			break;
		case 40:
			RainbowXoffset(.03);
			AmberSmatter(4);
			break;
		case 41:
			RainbowXoffset(.03);
			AmberSmatter(7);
			break;
		case 42:
			RainbowYoffset(.07);
			break;
		case 43:
			RainbowYoffset(.07);
			AmberSmatter(4);
			break;
		case 44:
			RainbowYoffset(.07); 
			AmberSmatter(7);
			break;
		case 45:
			RainbowXoffset(.07); 
			break;
		case 46:
			RainbowXoffset(.07); 
			AmberSmatter(4);
			break;
		case 47:
			RainbowXoffset(.07);	
			AmberSmatter(7);
			break;
		case 48:
			RainbowYoffset(.21);
			break;
		case 49:
			RainbowYoffset(.21);
			AmberSmatter(4);
			break;
		case 50:
			RainbowYoffset(.21);
			AmberSmatter(7);
			break;
		case 51:
			RainbowXoffset(.21);
			break;	
		case 52:
			RainbowXoffset(.21);
			AmberSmatter(4);
			break;
		case 53:
			RainbowXoffset(.21);
			AmberSmatter(7);
			break;
		case 54:
			break;	
		case 55:
			break;
		case 56:
			break;
		case 57:
			break;	
		case 58:
			
			break;
		case 59:
			break;
		case 60:
			break;
		case 61:
			
			break;
		case 62:
			
			break;
		case 63:

			break;
		case 64:

			break;
	}
}

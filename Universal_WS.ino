/***************************************************************************************
****************************************************************************************

Project: Large Amounts of WS281X Lights for Anything!

Description: Controls a WS281X based strands of LEDs. Accepts input from a Rotary
             Encoder (pattern select) and Potentiometer (brightness). Stores current 
             pattern in to memory for recall after power loss.

    
    
Change log: 

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
#define BRIGHT_FLOOR	0

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

#define maxPixels 		150		//Number of Pixels (add 1 extra for each level of upscaling)
#define UPSAMPLE 		3  		//1 = No Upsampling, up to maximum of 4
#define PIN_DATA 		6		//WS28XX data line

Adafruit_NeoPixel strip = Adafruit_NeoPixel(maxPixels, PIN_DATA, NEO_RGB + NEO_KHZ800);


/***************************************************************************************
 						>>>>>> END OF CONFIGURATION <<<<<<
***************************************************************************************/



/***************************************************************************************
  General Variables
***************************************************************************************/
int currBrightness = 0;
int tempValue = 0;
int brightnessValue = 0;
int speedValue = 0;
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

int upperLimit = 64; // max number of patterns (encoder)
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

	//Randomize the Simplex Noise values for lava lamp style patterns
	//Create a random seed by reading nearby electric noise on the analog ports
	randomSeed(analogRead(A6) + analogRead(A2) + analogRead(A3) + analogRead(A4) + analogRead (A5));
	
 	//Randomish x and y offset for simplex noise
 	yoffset = analogRead(A4) + analogRead (A5);
 	xoffset = analogRead(A5) + analogRead(A3);
	
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
}

/***************************************************************************************
  Main Loop
***************************************************************************************/
void loop()
{

	//SMOOTH OPERATOR
	brightnessTotal += analogRead(PIN_BRIGHT);
	brightnessCount += 1;

	if (SPEED_KNOB == 1)
  	{
		speedTotal += analogRead(PIN_SPEED);
		speedCount += 1;
	}

	//Get encoder button value
	GetEncoderButtonValue();

	//Framerate Control
	if (fpsMillis < millis())
  	{
		
		//1 second = 1000 millis. 1000 millis / 60 fps = 16 millis between frames
		fpsMillis = millis() + 16;

		//Read Potentiometer & Encoder
		readPots();

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
			//Store the encoder position
			EEPROM.write(1, encoderPos);
			//StorePattern();
			
			//Update the oldEncPos value to current
			oldEncPos = encoderPos;
		}

		// display appropriate pattern
		callColorFunction();

		//delay(10);
		
		//Transmit one frame of data out
		strip.show();
	}
}

void callColorFunction()
{
	// select color
  	switch (encoderPos-1) {
		case 0:
			Amber3();
			break;
		case 1:           //Single Color (1-6)
			Amber4();
			break;
		case 2:
			Amber5();
			break;
		case 3:
			WarmWhite(); //
			break;
		case 4:
			NuetralWhite(); //Amber 5 + a bit
			break;
		case 5:
			White(); //Warm White
			break;
		case 6:
			Green();
			break;
		case 7:
			Blue();
			break;
		case 8:           
			Teal();
			break;
		case 9:
			Purple();
			break;
		case 10:
			RainbowFlowFull();
			//rainbowFlag();
			break;
		case 11:
			SparkleBlueGreen();
			break;
		case 12:
			PurpleGoldSparkle();
			break;
		case 13://Two Colors=
			SteadyAlternatingColors(currBrightness,0,currBrightness/1.5, currBrightness, currBrightness/5, 0, (currSpeed + 1) / 6);
			break;
		case 14:
			SteadyAlternatingColors(currBrightness/2,currBrightness/2,currBrightness/3, currBrightness, currBrightness/4, 0, (currSpeed + 1) / 6);
			break;
		case 15:
			SteadyAlternatingColors(0,currBrightness,0, currBrightness, 0, 0, (currSpeed + 1) / 6);
			break;
		case 16:
			SteadyAlternatingColors(currBrightness,0,currBrightness/2, 0, 0, currBrightness, (currSpeed + 1) / 6);
			break;
		case 17:
			SteadyAlternatingColors(0,0,currBrightness, currBrightness/3,currBrightness/3,currBrightness/3, (currSpeed + 1) / 6);
			break;
		case 18:
			SteadyAlternatingColors(0,0,currBrightness, 0, currBrightness, 0, (currSpeed + 1) / 6);
			break;
		case 19:
			SteadyAlternatingColors(0,0,currBrightness, currBrightness, currBrightness/5, 0, (currSpeed + 1) / 6);
			break;
		case 20://Rainbow Flow Colors
			RainbowHsvFast();
			break;
		case 21:
			RainbowFlow();
			break;
		case 22:   
			DualColorFlow();
			break;
		case 23:   
			DualColorFlowFat();
			break;
		case 24:
			RainbowMovingPiece();
			break;
		case 25:
			RainbowHsvSlow();
			break;
		case 26:
			colorWipe();
			break;
		case 27:
			DualColorFlowGreenFast();
			break;
		case 28:
			DualColorFlowFast();
			break;
		case 29:
			sparkleRasta();
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
			GlowingAmberWhite();
			break;
		case 34:
			triFlag();
			break;
		case 35:
			rastaFlag();
			break;
		case 36:
			RainbowHsvTight();
			break;
		case 37:
			DualColorFlowFast2();
			break;
		case 38:
			DualColorFlowFast3();
			break;
		case 39:
			colorWipeBounce();
  			break;
		case 40:
			waterFlow();
			break;
		case 41:
			waterFlag();
			break;
		case 42:
			RainbowBigXoffset();
			break;
		case 43:
			RainbowBigXoffset();
			AmberSmatter(4);
			break;
		case 44:
			RainbowBigXoffset(); 
			AmberSmatter(6);
			break;
		case 45:
			RainbowBigXoffset();
			AmberSmatter(10);
			break;
		case 46:
			RainbowBigYoffset(); 
			break;
		case 47:
			RainbowBigYoffset();	
			AmberSmatter(3);
			break;
		case 48:
			RainbowBigYoffset(); 
			AmberSmatter(10);
			break;
		case 49:
			RainbowTwoYoffset();
			break;
		case 50:
			RainbowTwoYoffset();
			AmberSmatter(5);
			break;
		case 51:
			RainbowTwoYoffset();
			AmberSmatter(7);
			break;	
		case 52:
			RainbowFourXoffset();
			break;
		case 53:
			RainbowFourXoffset();
			AmberSmatter(2);
			break;
		case 54:
			RainbowFourXoffset();
			AmberSmatter(5);
			break;	
		case 55:
			RainbowOneYoffset();
			break;
		case 56:
			RainbowOneYoffset();
			AmberSmatter(4);
			break;
		case 57:
			RainbowOneYoffset();
			AmberSmatter(6);
			break;	
		case 58:
			RainbowThreeXoffset();
			break;
		case 59:
			RainbowThreeXoffset();
			AmberSmatter(3);
			break;
		case 60:
			RainbowThreeXoffset();
			AmberSmatter(6);
			break;
		case 61:
			GlowingAmber();
			break;
		case 62:
			Red();
			break;
		case 63:
			Amber();
			break;
		case 64:
			Amber2();
			break;
	}
}

/***************************************************************************************
****************************************************************************************

Project: Large Amounts of WS281X Lights for Anything!

Description: Controls a WS281X based strands of LEDs. Accepts input from a Rotary
             Encoder (pattern select) and Potentiometer (brightness). Stores current 
             pattern in to memory for recall after power loss.

    
    
Change log: 

Version 2.7 - Brian - Added some new patterns and removed others
	12/24/2022

Version 2.6 - Brian - Added option debug serial output
	7/10/2022

Version 2.5 - Brian - Actually fixed simplex noise turning white issue
	6/29/2022

Version 2.4 - Brian - Maybe finally fixed the simplex noise turning white issue
  	12/4/2021

Version 2.3 - Brian - Granular fade scaling, can be increased when using 100 pixels or less
	4/24/2021			Improved out of bounds rollover for Simplex Noise functions
						Speed adjustments / Pattern Updates

Version 2.2 - Brian - Slightly reduced white output for 12v regulated LEDs

Version 2.1 - Brian - Added dyanmic pixel upscaling and provisioning from 6 up to hardware max
	3/6/2021

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
							ported to WS2811 library at some point

Version Origin
   8/20/13 - Brian		- Burning Man 2013. 98 pixel LPD6803 control software "BUS 98"

Future Improvements:

  
****************************************************************************************
***************************************************************************************/

/***************************************************************************************
  Libraries
***************************************************************************************/
#include <Adafruit_NeoPixel.h>	//WS type strips
#include <EEPROM.h>        		//Persistent Memory


/***************************************************************************************
  Debugging Serial Monitor Output, 115200 baud
***************************************************************************************/
//#define DEBUG


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
#define BRIGHT_FLOOR	2		//Deadspace for Brightness knob, higher values create more space
#define SPEED_FLOOR		15		//Deadspace for Speed knob, lower values reduce space

#define SPACING_TRIPLE 	5.75	//5.75 for 50/100, 10 for 30
#define SPACING_DOUBLE 	4		//4 for 50/100, 6.5 for 30

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

  //Example for G50 Outdoor:
  Adafruit_NeoPixel strip = Adafruit_NeoPixel(HARDWARE_PIXELS, PIN_DATA, NEO_RBG + NEO_KHZ800);

***************************************************************************************/

#define HARDWARE_PIXELS		100		//Max supported # of Pixels
#define HARDWARE_UPSAMPLE	2		//Max Upsampling !!should not be adjusted!! Will auto adjust down.
#define FADE_UPSAMPLE		1		//Max Upsampling for Fade - 2 for >100 pixels, 1 for <100 
#define interfadeMax		8		//Fade time between patterns in frames
#define PIN_DATA 			6		//WS28XX data line

Adafruit_NeoPixel strip = Adafruit_NeoPixel(HARDWARE_PIXELS, PIN_DATA, NEO_RGB + NEO_KHZ800);

//byte ledTemp[(HARDWARE_PIXELS / HARDWARE_UPSAMPLE)][3];	
byte ledTemp[(HARDWARE_PIXELS / FADE_UPSAMPLE)][3]; //Compressed frame buffer if needed

/***************************************************************************************
 						>>>>>> END OF CONFIGURATION <<<<<<
***************************************************************************************/



/***************************************************************************************
  General Variables
***************************************************************************************/
int maxPixels = HARDWARE_PIXELS;
int UPSAMPLE = 3;
unsigned long settingsTimer = 0;
bool settingsMenu = 0;
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

#define MAX_PATTERNS 48
int lowerLimit = 1; // minimum number of patterns (encoder)
int encoderButton = 49;
byte favoritePattern = 2;
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
int LEDs_in_strip = 0; //defined in adjustSimplex();
const int LEDs_for_simplex = 6;

// Extra fake LED at the end, to avoid fencepost problem.
// It is used by simplex node and interpolation code.
float LED_array_red[(HARDWARE_PIXELS / HARDWARE_UPSAMPLE)+HARDWARE_UPSAMPLE];
float LED_array_green[(HARDWARE_PIXELS / HARDWARE_UPSAMPLE)+HARDWARE_UPSAMPLE];
float LED_array_blue[(HARDWARE_PIXELS / HARDWARE_UPSAMPLE)+HARDWARE_UPSAMPLE];
int node_spacing = (HARDWARE_PIXELS / HARDWARE_UPSAMPLE) / LEDs_for_simplex;

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
 	#ifdef DEBUG
 		Serial.begin(115200);
		Serial.println("Powering On");
	#endif

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

	//The value of these unconnected pins is used for random seed generation
	pinMode(A6, INPUT);
	pinMode(A7, INPUT);

	//Randomize the Simplex Noise values for lava lamp style patterns
	//Create a random seed by reading nearby electric noise on the analog ports
	randomSeed(analogRead(PIN_SPEED) + analogRead (PIN_BRIGHT) + analogRead(A6) + analogRead (A7));
	
 	//Randomish x and y offset for simplex noise
	checkOffset();
 	
	//xoffset = 15000;

	//Seed color
	h = random(500) / 1000.0;
    hOld = h + random(500) / 1000.0;

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

 	//Read max pixels and upscaling from memory
 	readMaxPixels();

	//Initialize up the LED strip
	strip.begin();
	
	//Display one frame to the the strip, to start they are all 'off'
	strip.show();

	//Read from the onboard persistant memory for last program position
	lastSavedEncoderPosition = EEPROM.read(1);
	
	if (lastSavedEncoderPosition > MAX_PATTERNS)
	{
		lastSavedEncoderPosition = 1;
	}

	#ifdef DEBUG
		Serial.print("Last Pattern: ");
		Serial.println(lastSavedEncoderPosition - 1);
	#endif
	
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
	if ((millis() - fpsMillis) >= 16)
	{
		//Check to see if we are in regular run mode
		if (settingsMenu == false)
		{
			//1 second = 1000 millis. 1000 millis / 60 fps = 16 millis between frames
			fpsMillis = millis();

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
				if (encoderPos > MAX_PATTERNS){
					encoderPos = lowerLimit;
				}
				if (encoderPos < lowerLimit) {
					encoderPos = MAX_PATTERNS;
				}

				//Start new interfade
				smoothFadeBegin();
				
				//Start countdown to store encoder setting to EEPROM
				writeDelay = 120;
				
				//Update the oldEncPos value to current
				oldEncPos = encoderPos;
	
				#ifdef DEBUG
					Serial.print("Pattern: ");
					Serial.println(encoderPos-1);
				#endif
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
		}		
		else //We are in the settings menu
		{
			setMaxPixelsMenu();
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
			//singleColor(currBrightness, (currBrightness/3) + (currBrightness * (float(currSpeed) / 400)), (currBrightness/12) + (currBrightness * (float(currSpeed) / 1200)));
			singleColor(currBrightness, (currBrightness/3) + (currBrightness * (float(currSpeed) / 375)), (currBrightness/12) + (currBrightness * (float(currSpeed) / 1000)));
			//singleColor(currBrightness, currBrightness/4, currBrightness/25);
			break;
		case 3:
			//Neutral White > Day Light
			//singleColor(currBrightness, (currBrightness/2.5) + (currBrightness * (float(currSpeed) / 300)), (currBrightness/8) + (currBrightness * (float(currSpeed) / 350)));
			singleColor(currBrightness, (currBrightness/2.5) + (currBrightness * (float(currSpeed) / 300)), (currBrightness/8) + (currBrightness * (float(currSpeed) / 420)));
			//singleColor(currBrightness, currBrightness/3.5,currBrightness/15);
			break;
		case 4:
			//Day Light > Cool Light
			singleColor(currBrightness, (currBrightness/1.75) + (currBrightness * (float(currSpeed) / 250)), (currBrightness/3.5) + (currBrightness * (float(currSpeed) / 275)));
			//singleColor(currBrightness * 0.9, (currBrightness/2) + (currBrightness * (float(currSpeed) / 300)), (currBrightness/4) + (currBrightness * (float(currSpeed) / 250)));
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
			SteadyAlternatingColors(0,0,currBrightness, currBrightness, currBrightness/5, 0, (currSpeed + 1) / SPACING_DOUBLE);
			//SparkleBlueGreen();
			break;
		case 12:
			//Blue and Green
			SteadyAlternatingColors(0,0,currBrightness, 0, currBrightness, 0, (currSpeed + 1) / SPACING_DOUBLE);
			break;
		case 13:
			//Purple and gold
			SteadyAlternatingColors(currBrightness,0,currBrightness/1.5, currBrightness, currBrightness/3, 0, (currSpeed + 1) / SPACING_DOUBLE);
			break;
		case 14:
			//White and Amber
			SteadyAlternatingColors(currBrightness/2,currBrightness/2,currBrightness/3, currBrightness, currBrightness/4, 0, (currSpeed + 1) / SPACING_DOUBLE);
			break;
		case 15:
			//Green and Red
			SteadyAlternatingColors(0,currBrightness,0, currBrightness, 0, 0, (currSpeed + 1) / SPACING_DOUBLE);
			break;
		case 16:
			//Pink and Blue
			SteadyAlternatingColors(currBrightness,0,currBrightness/2, 0, 0, currBrightness, (currSpeed + 1) / SPACING_DOUBLE);
			break;
		case 17:
			//Blue and White
			SteadyAlternatingColors(0,0,currBrightness, currBrightness/2,currBrightness/2,currBrightness/2, (currSpeed + 1) / SPACING_DOUBLE);
			break;
		case 18:
			//Red and White
			SteadyAlternatingColors(currBrightness,0,0, currBrightness/2,currBrightness/2,currBrightness/2, (currSpeed + 1) / SPACING_DOUBLE);
			break;
		case 19:
			//Green and White
			SteadyAlternatingColors(0,currBrightness,0, currBrightness/2,currBrightness/2,currBrightness/2, (currSpeed + 1) / SPACING_DOUBLE);
			break;
		case 20:
			//Green and Gold
     	 	SteadyAlternatingColors(0,currBrightness * 1,0, currBrightness, currBrightness * .33, currBrightness * .00, (currSpeed + 1) / SPACING_DOUBLE);
			break;
		case 21:
			//Teal and Bronze
      		SteadyAlternatingColors(0,currBrightness * 1,currBrightness * .42, currBrightness, currBrightness * .33, currBrightness * .05, (currSpeed + 1) / SPACING_DOUBLE);
			break;
		case 22:
			//Whole strip go through single HSV colors
			RainbowHsv();
			break;
		case 23:
			//1/2 Rainbow
			RainbowFlow(float(0.5 / maxPixels));
			break;
		case 24:   
			//Full Rainbow
			RainbowFlow(float(1.0 / maxPixels));
			
			//DualColorFlowFat(0.2);
			//DualColorFlow();
			break;
		case 25:   
			
			triFlag();
			break;
		case 26:
			DualColorFlowFat(0.5);
			//DualColorFlowFat2();
			break;
		case 27:
			DualColorFlowFat(0.667);
			//DualColorFlowFat3();
			break;
		case 28:
			DualColorFlowFat(0.334);
			break;
		case 29:
			colorWipeBounce();
			//DualColorFlowGreenFast();
			break;
		case 30:
			colorWipe();
			//waterFlow();
			break;
		case 31:
			//Valentines Day
			//White, Pink, and Purple
      		SteadyAlternatingColorsThree(currBrightness/2,currBrightness/2,currBrightness/2, currBrightness,0,currBrightness / 8, currBrightness, 0, currBrightness , (currSpeed + 1) / SPACING_TRIPLE);
			break;
		case 32:
			//St Patrick's Day
      		//Green, Gold, and White
      		SteadyAlternatingColorsThree(0,currBrightness,0, currBrightness,currBrightness/3,currBrightness / 50, currBrightness / 2, currBrightness / 2, currBrightness / 2, (currSpeed + 1) / SPACING_TRIPLE);
			break;
		case 33:
			//Easter
			//Teal, Yellow, and Purple
      		SteadyAlternatingColorsThree(0,currBrightness,currBrightness/2, currBrightness,currBrightness / 3, 0, currBrightness, 0, currBrightness, (currSpeed + 1) / SPACING_TRIPLE);
			break;
		case 34:
			//April 20th
			//Red, Yellow, and Green
			SteadyAlternatingColorsThree(currBrightness,0,0, currBrightness,currBrightness/4,0, 0, currBrightness, 0, (currSpeed + 1) / SPACING_TRIPLE);
			break;
		case 35:
			//Father's Day
			//Blue, White, and Teal
			SteadyAlternatingColorsThree(0,0,currBrightness, currBrightness/2,currBrightness/2,currBrightness/2, 0, currBrightness / 2, currBrightness / 2, (currSpeed + 1) / SPACING_TRIPLE);
			break;
		case 36:
			//July 4th
			//Red, White, and Blue
			SteadyAlternatingColorsThree(currBrightness,0,0, currBrightness/2,currBrightness/2,currBrightness/2, 0, 0, currBrightness, (currSpeed + 1) / SPACING_TRIPLE);
			break;
		case 37:
			//Maryland
      		//Red, Yellow, and White
      		SteadyAlternatingColorsThree(currBrightness,0,0, currBrightness,currBrightness/5,0, currBrightness / 2, currBrightness / 2, currBrightness / 2, (currSpeed + 1) / SPACING_TRIPLE);			
			break;
		case 38:
			//Halloween
			//Purple, Orange, and Green
			SteadyAlternatingColorsThree(currBrightness,0,currBrightness / 3, currBrightness,currBrightness/7,0, 0, currBrightness, 0, (currSpeed + 1) / SPACING_TRIPLE);
			break;
		case 39:
			//Autumn
			//Red, Orange, and Yellow
			SteadyAlternatingColorsThree(currBrightness,0,0, currBrightness,currBrightness/7,0, currBrightness, currBrightness / 2, 0, (currSpeed + 1) / SPACING_TRIPLE);
			break;
		case 40:
			//Christmas
			//Red, White, and Green
			SteadyAlternatingColorsThree(currBrightness,0,0, currBrightness/2,currBrightness/2,currBrightness/2, 0, currBrightness, 0, (currSpeed + 1) / SPACING_TRIPLE);			
			break;
		case 41:
			//Christmas Lights
			christmasLights();
			effectFunction();
			break;
		case 42:
			//Tibet Lights
			tibetLights();
			effectFunction();
			
			break;
		case 43:
			GlowingAmber();
			//RainbowXoffset(.03);
			//AmberSmatter(7);
			break;
		case 44:
			RainbowYoffset(.21);
			break;
		case 45:
			RainbowYoffset(.21);
			AmberSmatter(4);
			break;
		case 46:
			RainbowXoffset(.21);
			break;
		case 47: 
			RainbowXoffset(.21);
			AmberSmatter(4);
			break;
		case 48:
			break;
		case 49:
			break;
		case 50:
			
			break;
		case 51:
     		
			break;
		case 52:
      		RainbowYoffset(.21);
			AmberSmatter(7);
			break;
		case 53:
     		//User this one
			
			break;	
		case 54:
      		//Use this one
			
			
			break;
		case 55:
			RainbowXoffset(.21);
			AmberSmatter(7);
			break;
	}
}

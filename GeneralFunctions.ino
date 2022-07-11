/***************************************************************************************
  Brightness
***************************************************************************************/
void smoothOperator(){
  
  //tempValue = analogRead(A1);

  tempValue = brightnessTotal / brightnessCount;
  brightnessCount = 0;
  brightnessTotal = 0;
  
  //filter noise
  if (((tempValue > brightnessValue + SENSITIVITY) || (tempValue + SENSITIVITY < brightnessValue)))
  {
    //check minimum value 
    brightnessValue = tempValue;
    tempValue = constrain(tempValue, BRIGHT_FLOOR, 1005);

    if (INVERT_BRIGHT == 1)
    {
      tempValue = map(tempValue, BRIGHT_FLOOR, 1005, 255, 0);
    }else
    {
      tempValue = map(tempValue, BRIGHT_FLOOR, 1005, 0, 255);
    }

    currBrightness = tempValue;

    #ifdef DEBUG
      Serial.print("Brightness: ");
      Serial.println(currBrightness);
    #endif
  };
  
  if (SPEED_KNOB == 1)
  {
    tempValue = speedTotal / speedCount;
    speedCount = 0;
    speedTotal = 0;

    if (INVERT_SPEED == 1)
    {
      tempValue = map(tempValue, 1023, 0, 0, 1023);
    }

    if (((tempValue > speedValue + SENSITIVITY * 2) || (tempValue + SENSITIVITY * 2 < speedValue)))
    {
      speedValue = tempValue;

      tempValue = constrain(tempValue, SPEED_FLOOR, 1005);
      tempValue = map(tempValue, SPEED_FLOOR, 1005, 100, 0);
      currSpeed = tempValue;

      #ifdef DEBUG
        Serial.print("Speed: ");
        Serial.println(currSpeed);
      #endif
    }
    
  }
}

void smoothFade(byte fade)
{
  for (int i = 0; i < maxPixels; i++)
  {
    red = (float(ledTemp[i / FADE_UPSAMPLE][0] / 255.0) * fade) + ((float(strip.getPixelColor(i) >> 16 & 0xff) / 255.0) * (255 - fade));
    green = (float(ledTemp[i / FADE_UPSAMPLE][1] / 255.0) * fade) + ((float(strip.getPixelColor(i) >> 8 & 0xff) / 255.0) * (255 - fade));
    blue = (float(ledTemp[i / FADE_UPSAMPLE][2] / 255.0) * fade) + ((float(strip.getPixelColor(i) & 0xff) / 255.0) * (255 - fade));

    strip.setPixelColor(i, red, green, blue);
  }  
}

void smoothFadeBegin()
{
  interfade = interfadeMax;

  //Copy current LED values to temp for fading
  for (int i = 0; i < (maxPixels / FADE_UPSAMPLE); i++)
  {	
    red = 0;
    green = 0;
    blue = 0;

    //Add values of lights for each FADE_UPSAMPLE range
    for (int u = 0; u < FADE_UPSAMPLE; u++)
    {
      red += (strip.getPixelColor(u + i * FADE_UPSAMPLE) >> 16 & 0xff);
      green += ((strip.getPixelColor(u + i * FADE_UPSAMPLE) >> 8) & 0xff);
      blue += (strip.getPixelColor(u + i * FADE_UPSAMPLE) & 0xff);
    }

    //Store the values, averaged if FADE_UPSAMPLEd
    ledTemp[i][0] = red / FADE_UPSAMPLE;
    ledTemp[i][1] = green / FADE_UPSAMPLE;
    ledTemp[i][2] = blue / FADE_UPSAMPLE;
  }
}

int fastfloor(float n) {
  return n > 0 ? (int) n : (int) n - 1;
}
float k_fn(int a) {
  s = (A[0] + A[1] + A[2]) * onesixth;
  float x = u - A[0] + s;
  float y = v - A[1] + s;
  float z = w - A[2] + s;
  float t = 0.6f - x * x - y * y - z * z;
  int h = shuffle(i + A[0], j + A[1], k + A[2]);
  A[a]++;
  if (t < 0) return 0;
  int b5 = h >> 5 & 1;
  int b4 = h >> 4 & 1;
  int b3 = h >> 3 & 1;
  int b2 = h >> 2 & 1;
  int b = h & 3;
  float p = b == 1 ? x : b == 2 ? y : z;
  float q = b == 1 ? y : b == 2 ? z : x;
  float r = b == 1 ? z : b == 2 ? x : y;
  p = b5 == b3 ? -p : p;
  q = b5 == b4 ? -q: q;
  r = b5 != (b4^b3) ? -r : r;
  t *= t;
  return 8 * t * t * (p + (b == 0 ? q + r : b2 == 0 ? q : r));
}
int shuffle(int i, int j, int k) {
  return b(i, j, k, 0) + b(j, k, i, 1) + b(k, i, j, 2) + b(i, j, k, 3) + b(j, k, i, 4) + b(k, i, j, 5) + b(i, j, k, 6) + b(j, k, i, 7);
}
int b(int i, int j, int k, int B) {
  return T[b(i, B) << 2 | b(j, B) << 1 | b(k, B)];
}
int b(int N, int B) {
  return N >> B & 1;
}


void effectFunction()
{
  if (currSpeed > 2)
	{
	  //Load starting fade based on currFadeStep
		int tempStep = currFadeStep;

		for (int i = maxPixels; i > -1; i--)
		{
			// get the current colour
			uint32_t curr_col = strip.getPixelColor(i);

			// do some bitshifting
			red3 = (curr_col >> 16) & 0xFF;
			green3 = (curr_col >> 8) & 0xFF;
			blue3 = curr_col & 0xFF;

			switch(tempStep)
			{
        /* Sine Wave
				case 0:
					//no change
					break;
				case 1:
					fadeEffect(1.5, red3, green3, blue3);
					break;
				case 2:
					fadeEffect(2, red3, green3, blue3);
					break;
				case 3:
					fadeEffect(2.5, red3, green3, blue3);
					break;
				case 4:
					fadeEffect(3, red3, green3, blue3);
					break;
				case 5:
					fadeEffect(3.5, red3, green3, blue3);
					break;
				case 6:
					fadeEffect(4, red3, green3, blue3);
					break;
				case 7:
					fadeEffect(4.5, red3, green3, blue3);
					break;
				case 8:
					fadeEffect(5, red3, green3, blue3);
					break;
				case 9:
					fadeEffect(5.5, red3, green3, blue3);
					break;
				case 10:
					fadeEffect(6, red3, green3, blue3);
					break;
				case 11:
					fadeEffect(5.5, red3, green3, blue3);
					break;
				case 12:
					fadeEffect(5, red3, green3, blue3);
					break;
				case 13:
					fadeEffect(4.5, red3, green3, blue3);
					break;
				case 14:
					fadeEffect(4, red3, green3, blue3);
					break;
				case 15:
					fadeEffect(3.5, red3, green3, blue3);
					break;
				case 16:
					fadeEffect(3, red3, green3, blue3);
					break;
				case 17:
					fadeEffect(2.5, red3, green3, blue3);
					break;
				case 18:
					fadeEffect(2, red3, green3, blue3);
					break;
				case 19:
					fadeEffect(1.5, red3, green3, blue3);
					break;
        */
       
       // Pulse
      	case 0:
					//no change
				  break;
				case 1:
					fadeEffect(1.25, red3, green3, blue3);
					break;
				case 2:
					fadeEffect(1.5, red3, green3, blue3);
					break;
				case 3:
					fadeEffect(1.75, red3, green3, blue3);
					break;
				case 4:
					fadeEffect(2, red3, green3, blue3);
					break;
				case 5:
					fadeEffect(2.25, red3, green3, blue3);
					break;
				case 6:
					fadeEffect(2.50, red3, green3, blue3);
					break;
				case 7:
					fadeEffect(2.75, red3, green3, blue3);
					break;
				case 8:
					fadeEffect(3, red3, green3, blue3);
					break;
				case 9:
					fadeEffect(3.5, red3, green3, blue3);
					break;
				case 10:
					fadeEffect(4, red3, green3, blue3);
					break;
				case 11:
					fadeEffect(4.5, red3, green3, blue3);
					break;
				case 12:
					fadeEffect(5, red3, green3, blue3);
					break;
				case 13:
					fadeEffect(5.5, red3, green3, blue3);
					break;
				case 14:
					fadeEffect(6, red3, green3, blue3);
					break;
				case 15:
					fadeEffect(6.5, red3, green3, blue3);
					break;
				case 16:
					fadeEffect(8, red3, green3, blue3);
					break;
				case 17:
					fadeEffect(8.5, red3, green3, blue3);
					break;
				case 18:
					fadeEffect(9, red3, green3, blue3);
					break;
				case 19:
					fadeEffect(10, red3, green3, blue3);
					break;
			}

			//incremement tempStep to prepare for next pixel & catch any rollover
			tempStep += 1;
			if (tempStep > 19)
			{
				tempStep = 0;
			}

			// set the color of the pixel
			strip.setPixelColor(i, strip.Color(int(red3), int(green3), int(blue3)));
		}

    if (effectMillis < millis()){
      effectMillis = millis() + ((120 - currSpeed) * 10);
      //delay(255 - currSpeed);
      //incremement currFadeStep to prepare for next frame & catch any rollover
      currFadeStep += 1;
      if (currFadeStep > 19)
      {
        currFadeStep = 0;
      }
    } 
	} 
}

void fadeEffect(float fadeAmount, int& R, int& G, int& B)
{
  R = R / fadeAmount;
  G = G / fadeAmount;
  B = B / fadeAmount;
}

/***************************************************************************************
  Encoder / Analog Inputs
***************************************************************************************/
void readInputs(){

  //Analog input running averages and counts, these are processed in Smooth Operator
  brightnessTotal += analogRead(PIN_BRIGHT);
	brightnessCount += 1;

	if (SPEED_KNOB == 1)
  {
		speedTotal += analogRead(PIN_SPEED);
		speedCount += 1;
	}

  //Check for button presses each time. This makes sure we don't miss one / feels responsive

  encoderButton = digitalRead(4); // read digital pin 4
  
  //Is encoder button being pressed?
  if (encoderButton == 0)
  {
    if (settingsMenu == 1)
    {
      //Disable menu
      settingsMenu = 0;

      //Adjust simplex variables depending on new settings
      adjustSimplex();

      //Write it to memory
      EEPROM.write(2, maxPixels);
      EEPROM.write(3, UPSAMPLE);
    }

    if (settingsTimer == 0)
    {
      settingsTimer = millis();
    }
    //Has it been held down for ~4 seconds?
    
    if (settingsTimer != 0)
    {
      if (settingsTimer + 3000 < millis())
      {
        if (encoderPos != favoritePattern)
        {
          float tempHue = 0;

          
          for (int i = 0; i < maxPixels; i++)
          {
            tempHue = tempHue + (1.0 / maxPixels);

            if (tempHue > 1)
            {
              tempHue = tempHue - 1;
            }

            hsv2rgb(tempHue, 1, float(currBrightness / 255.0), red, green, blue);
            strip.setPixelColor(i, strip.Color(red,green,blue));
            strip.show();

            delay(5);

            //Let go off button?
            if (digitalRead(4) == true)
            {
              i = maxPixels; //Exit for loop
            }
          }

          //Still holding button? Save it!
          if (digitalRead(4) == false)
          {
            favoritePattern = encoderPos;
            EEPROM.write(4, favoritePattern);

            //Create a blink to let user know
            singleColor(0, 0, 0);
            strip.show();
            delay(500);
            smoothFadeBegin();
          }

        }
      }

      //Has it been held down ~10 seconds?
      if (settingsTimer + 10000 < millis())
      {
        //Enable max pixel menu
        settingsMenu = 1;

        //Reset timer
        settingsTimer = 0;

        //Place us based on current max pixel setting
        encoderPos = maxPixels;

        //Show Christmas lights to signify menu entry
        christmasLights();
        strip.show();

        //Wait 2 seconds
        delay(2000);
      }
    }
  }
  else //button is not being held
  {
    //Has the timer gone over 0? Change to favorite pattern
    if (settingsTimer > 0)
    {
      //Set pattern back to user favorite
      encoderPos = favoritePattern;
      
      settingsTimer = 0;
    }
  }
}


/***************************************************************************************
  Settings Menus
***************************************************************************************/
void setMaxPixelsMenu()
{
  //Set output to Red by default, and Upsample rate to 1x
	red = 255;
	green = 0;
	blue = 0;
	UPSAMPLE = 1;

  //Make sure we don't go too high
  if (encoderPos > HARDWARE_PIXELS)
  {
		encoderPos = HARDWARE_PIXELS;
	}
  //Make sure we don't go too low, some issues with simplex noise
	if (encoderPos < 6)
	{
		encoderPos = 6;
	}

  //Set the system Max Pixels to the currently select amount
	maxPixels = encoderPos;
	
  //Check to see if we need to start upscaling
	if (encoderPos > (HARDWARE_PIXELS / HARDWARE_UPSAMPLE))
	{
    //We do, 2x
		red = 0;
		green = 255;
		blue = 0;
		UPSAMPLE = 2;
	}

  //See if even more upscaling is needed
	if (encoderPos > (HARDWARE_PIXELS / HARDWARE_UPSAMPLE) * 2)
	{
    //it is, 3x 
		red = 0;
		green = 0;
		blue = 255;
		UPSAMPLE = 3;
	}

  //Draw the current amount of pixels
	for(uint16_t i=0; i<maxPixels; i++)
  {
		strip.setPixelColor(i, strip.Color(red,green,blue));
	}

  //Clear off any that have been deselected
	for(uint16_t i=maxPixels; i<HARDWARE_PIXELS; i++)
  {
		strip.setPixelColor(i, strip.Color(0,0,0));
	}

  //Accent currently selected
  strip.setPixelColor(maxPixels - 1, strip.Color(255,255,255));

  //Strip output is in main function
}

void readMaxPixels()
{
  maxPixels = EEPROM.read(2);
	UPSAMPLE = EEPROM.read(3);
  favoritePattern = EEPROM.read(4);

	if (maxPixels > HARDWARE_PIXELS)
	{
		maxPixels = HARDWARE_PIXELS;
	}

	if (UPSAMPLE > HARDWARE_UPSAMPLE)
	{
		UPSAMPLE = HARDWARE_UPSAMPLE;
	}

	if ((maxPixels / UPSAMPLE) > (HARDWARE_PIXELS / HARDWARE_UPSAMPLE))
	{
		maxPixels = 50;
		UPSAMPLE = 1;
	}

  if (favoritePattern > MAX_PATTERNS)
  {
    favoritePattern = 2; //Warm White by default
  }

  #ifdef DEBUG
    Serial.print("Pixels: ");
    Serial.println(maxPixels);
    Serial.print("Upsample: ");
    Serial.println(UPSAMPLE);
    Serial.print("Favorite Pattern: ");
    Serial.println(favoritePattern - 1);
	#endif

	//Adjust simplex variables depending on settings
  adjustSimplex();
}

void adjustSimplex()
{
  LEDs_in_strip = (maxPixels / UPSAMPLE) + UPSAMPLE;
	node_spacing = LEDs_in_strip / LEDs_for_simplex;
}

void PinA(){
  //Commented out cli and sei for PinA and PinB due to suspected crashing issue
  //cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; // read all eight pin values then strip away all but pinA and pinB's values
  if(reading == B00001100 && aFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos --; //decrement the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00000100) bFlag = 1; //signal that we're expecting pinB to signal the transition to detent from free rotation
  //sei(); //restart interrupts
}

void PinB(){

  //cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; //read all eight pin values then strip away all but pinA and pinB's values
  if (reading == B00001100 && bFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoderPos ++; //increment the encoder's position count
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00001000) aFlag = 1; //signal that we're expecting pinA to signal the transition to detent from free rotation
  //sei(); //restart interrupts
}

/***********************************************************
  hsv2rgb
  Function: Calculate RGB values for colors represented
    in Hue, Saturation, and Value (brightness).
***********************************************************/
void hsv2rgb(float H, float S, float V, int& R, int& G, int& B)
{
  int var_i;
  float var_1, var_2, var_3, var_h, var_r, var_g, var_b;
  if ( S == 0 )                       //HSV values = 0 ÷ 1
  {
    R = V * 255;
    G = V * 255;
    B = V * 255;
  }
  else
  {
    var_h = H * 6;
    if ( var_h == 6 ) var_h = 0;      //H must be < 1
    var_i = int( var_h ) ;            //Or ... var_i = floor( var_h )
    var_1 = V * ( 1 - S );
    var_2 = V * ( 1 - S * ( var_h - var_i ) );
    var_3 = V * ( 1 - S * ( 1 - ( var_h - var_i ) ) );

    if ( var_i == 0 ) {
    var_r = V     ;
    var_g = var_3 ;
    var_b = var_1 ;
    }
    else if ( var_i == 1 ) {
    var_r = var_2 ;
    var_g = V     ;
    var_b = var_1 ;
    }
    else if ( var_i == 2 ) {
    var_r = var_1 ;
    var_g = V     ;
    var_b = var_3 ;
    }
    else if ( var_i == 3 ) {
    var_r = var_1 ;
    var_g = var_2 ;
    var_b = V     ;
    }
    else if ( var_i == 4 ) {
    var_r = var_3 ;
    var_g = var_1 ;
    var_b = V     ;
    }
    else                   {
    var_r = V     ;
    var_g = var_1 ;
    var_b = var_2 ;
    }

    //RGB results = 0 ÷ 255 (Production)
    R = (var_r) * 255;
    G = (var_g) * 255;
    B = (var_b) * 255;
  }
}

/*****************************************************************************/
// Simplex noise code:
// From an original algorythm by Ken Perlin.
// Returns a value in the range of about [-0.347 .. 0.347]
float SimplexNoise(float x, float y, float z) {
  // Skew input space to relative coordinate in simplex cell
  s = (x + y + z) * onethird;
  i = fastfloor(x+s);
  j = fastfloor(y+s);
  k = fastfloor(z+s);
   
  // Unskew cell origin back to (x, y , z) space
  s = (i + j + k) * onesixth;
  u = x - i + s;
  v = y - j + s;
  w = z - k + s;;
   
  A[0] = A[1] = A[2] = 0;
   
  // For 3D case, the simplex shape is a slightly irregular tetrahedron.
  // Determine which simplex we're in
  int hi = u >= w ? u >= v ? 0 : 1 : v >= w ? 1 : 2;
  int lo = u < w ? u < v ? 0 : 1 : v < w ? 1 : 2;
   
  return k_fn(hi) + k_fn(3 - hi - lo) + k_fn(lo) + k_fn(0);
}



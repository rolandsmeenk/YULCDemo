#include <Arduino.h>
#include <FastLED.h>

// YULC board pin definitions
#define BOOTBUTTON_PIN 0 	// GPIO pin for the boot button
#define DATA1_PIN 1 		// GPIO pin for the first LED data line
#define DATA2_PIN 2 		// GPIO pin for the second LED data line
#define MOSFET1_PIN 47 		// GPIO pin for the first MOSFET
#define MOSFET2_PIN 21 		// GPIO pin for the second MOSFET
#define FUSE_SENSE_PIN 7 	// GPIO pin for the fuse sense (AI) input

#define DEFAULT_BRIGHTNESS 32 // Max is 255, 32 is a conservative value to not overload a USB power supply (500mA) for 12x12 pixels.
#define NUM_LEDS 10    // Number of LEDs in your strip

CRGB leds[NUM_LEDS];

int BRIGHTNESS	= DEFAULT_BRIGHTNESS; // 32 - Max is 255, 32 is a conservative value to not overload a USB power supply (500mA) for 12x12 pixels.
unsigned long previousMillis = 0; // Stores the last time the LED was updated
const long interval = 100;        // Interval at which to move the pixel (milliseconds)
int currentPixel = 0;             // Tracks which pixel is currently lit
CRGB currentColor = CRGB::Green;  // Start with blue as default color

void ledAnimationLoop();
void temperatureCheckLoop();
void fuseCheckLoop();
void bootButtonCheckLoop();

void setup() 
{
 	Serial.begin(115200);
	while (!Serial);      // Wait for serial port to connect. Needed for native USB port boards
	
	Serial.println("*** Start setup ***");

	Serial.println("Activating MOSFETs for YULC channels");
  	pinMode(MOSFET1_PIN , OUTPUT);    	// switch on MOSFET for channel 1
  	digitalWrite(MOSFET1_PIN, HIGH);  	// switch on MOSFET for channel 1
  	pinMode(MOSFET2_PIN, OUTPUT);		// switch on MOSFET for channel 2
  	digitalWrite(MOSFET2_PIN, HIGH); 	// switch on MOSFET for channel 2

	FastLED.addLeds<WS2812B, DATA2_PIN, GRB>(leds, NUM_LEDS); // Adjust LED type and color order if needed
	FastLED.setBrightness(BRIGHTNESS); 	// Set a comfortable brightness level (0-255)
	FastLED.clear();           			// Clear all LEDs initially
	FastLED.show();

	Serial.println("Setup ready");
}

void loop() 
{
    ledAnimationLoop();

    temperatureCheckLoop();

    fuseCheckLoop();

	bootButtonCheckLoop();
}

void ledAnimationLoop()
{
    unsigned long currentMillis = millis(); // Get the current time

    // Check if enough time has passed since the last pixel update
    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis; // Save the current time for the next comparison

        // 1. Turn off the previous pixel
        leds[currentPixel] = CRGB::Black;

        // 2. Move to the next pixel
        currentPixel++;
        if (currentPixel >= NUM_LEDS)
        {
            currentPixel = 0; // Loop back to the beginning if we reached the end
        }

        // 3. Light up the new pixel with the current selected color
        leds[currentPixel] = currentColor; // Use the currentColor variable

        // 4. Update the LED strip
        FastLED.show();
    }
}

unsigned long lastTempCheck = 0;
// Check the board temperature every 5 seconds
void temperatureCheckLoop()
{
    if (millis() - lastTempCheck >= 5000)
    {
        lastTempCheck = millis();
		
		float boardTemp = (float)temperatureRead(); // Assuming temperatureRead() returns the temperature in Celsius
		Serial.print("Board temperature: ");
		Serial.print(boardTemp);
		Serial.println(" °C");

		// Shutdown if temperature exceeds 70°C
		if (boardTemp > 70.0)
		{
			Serial.println("Board temperature exceeded 70°C, shutting down...");
            shutdownLedOutput();
		}
    }
}

void shutdownLedOutput()
{
    FastLED.clear(); // Clear the LEDs
    FastLED.show();
    digitalWrite(MOSFET1_PIN, LOW); // Switch off MOSFET for channel 1
    digitalWrite(MOSFET2_PIN, LOW); // Switch off MOSFET for channel 2
}

unsigned long lastFuseCheck = 0;
void fuseCheckLoop()
{
	if (millis() - lastFuseCheck >= 5000) // Check every 5 seconds
	{
		lastFuseCheck = millis();
		int fuseValue = analogRead(FUSE_SENSE_PIN); // Read the fuse sense pin
		Serial.print("Fuse sense value: ");
		Serial.println(fuseValue);

		// Example logic to determine if the fuse is blown
		if (fuseValue < 100) // Adjust threshold as needed
		{
			Serial.println("Fuse is blown!");
			shutdownLedOutput();		
		}
	}
}

unsigned long lastButtonCheck = 0;
bool previousButtonState = HIGH; // Assume button is not pressed initially
void bootButtonCheckLoop()
{
	if (millis() - lastButtonCheck >= 100) // Check every 100 milliseconds
	{
		lastButtonCheck = millis();
		bool currentButtonState = digitalRead(BOOTBUTTON_PIN); // Read the button state
		if (currentButtonState != previousButtonState) // Button state changed
		{
			if (currentButtonState == LOW) // Button pressed
			{
				// Serial.println("Boot button pressed, performing action...");
				// Perform the desired action here, e.g., reset the device or change LED color
				if (currentColor == CRGB::Green) 
				{
					currentColor = CRGB::Blue; // Change color to blue as an example
				} 
				else 
				{
					currentColor = CRGB::Green; // Change color to green as an example
				}
			}
			previousButtonState = currentButtonState; // Update the previous state
		}
	}
}



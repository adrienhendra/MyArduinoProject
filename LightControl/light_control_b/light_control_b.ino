/*
* My light control
* Date: September 16 2014
* Written by Adrien
*/

#include <Keypad.h>
#include <Wire.h>  //I2C Master
#include "pitches.h"  //Pitch table

//LEDs
const int LED_R = 9;
const int LED_G = 5;
const int LED_B = 6;

//Keypad
const byte KEYPAD_R1 = 4;
const byte KEYPAD_R2 = 7;
const byte KEYPAD_R3 = 8;
const byte KEYPAD_R4 = 3;
const byte KEYPAD_C1 = 10;
const byte KEYPAD_C2 = 11;
const byte KEYPAD_C3 = 12;
const byte KEYPAD_ROWS = 4;
const byte KEYPAD_COLS = 3;

//Speaker
const byte SPEAKER_A = 2;

char keys[KEYPAD_ROWS][KEYPAD_COLS] =
{
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'#', '0', '*'}
};

byte rowPins[KEYPAD_ROWS] = {KEYPAD_R1, KEYPAD_R2, KEYPAD_R3, KEYPAD_R4};
byte colPins[KEYPAD_COLS] = {KEYPAD_C1, KEYPAD_C2, KEYPAD_C3};

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, KEYPAD_ROWS, KEYPAD_COLS );

static byte LED_VAL_R = 0;
static byte LED_VAL_G = 0;
static byte LED_VAL_B = 0;

void setup()
{
  //Clear LED states
  LED_VAL_R = 0;
  LED_VAL_G = 0;
  LED_VAL_B = 0;

  //Initialize I2C
  Wire.begin();  //Initialize connection as Master

  //Enable Serial
  Serial.begin(9600);

  //Set PWM output
  pinMode(LED_R, OUTPUT);      // sets the digital pin as output
  pinMode(LED_G, OUTPUT);      // sets the digital pin as output
  pinMode(LED_B, OUTPUT);      // sets the digital pin as output

  //Starting application
  Serial.println("Starting LED controller board!");
}

void loop()
{
  //Keypad stuff
  char key = keypad.getKey();

  if (key != NO_KEY)
  {
    ReadKey(key);
    UpdateLEDState();
  }

  //Delay
  delay(10);
}

void ReadKey(char key)
{
  Serial.println(key);
  switch (key)
  {
    case '1':
      LED_VAL_R += 5;
      break;
    case '2':
      LED_VAL_G += 5;
      break;
    case '3':
      LED_VAL_B += 5;
      break;
    case '4':
      LED_VAL_R -= 5;
      break;
    case '5':
      LED_VAL_G -= 5;
      break;
    case '6':
      LED_VAL_B -= 5;
      break;
    case '7':
      Beep(NOTE_C3, 100);
      break;
    case '8':
      Beep(NOTE_D3, 100);
      break;
    case '9':
      Beep(NOTE_E3, 100);
      break;
    case '#':
      SendBrightness(0);
      break;
    case '0':
      SendBrightness(1024);
      break;
    case '*':
      SendBrightness(4095);
      break;
    default:
      Serial.println("??Key not assigned??");
      break;
  }
}

void UpdateLEDState()
{
  //Check RGB max min
  if (LED_VAL_R < 0) LED_VAL_R = 0;
  if (LED_VAL_R > 255) LED_VAL_R = 255;
  if (LED_VAL_G < 0) LED_VAL_G = 0;
  if (LED_VAL_G > 255) LED_VAL_G = 255;
  if (LED_VAL_B < 0) LED_VAL_B = 0;
  if (LED_VAL_B > 255) LED_VAL_B = 255;

  //Change value
  analogWrite(LED_R, LED_VAL_R );
  analogWrite(LED_G, LED_VAL_G);
  analogWrite(LED_B, LED_VAL_B);

  //Print value
  Serial.print(">> R:");
  Serial.print(LED_VAL_R, DEC);
  Serial.print(">> G:");
  Serial.print(LED_VAL_G, DEC);
  Serial.print(">> B:");
  Serial.print(LED_VAL_B, DEC);
  Serial.print("\r\n");
}

void Beep(unsigned int beep_tone, unsigned long beep_ms)
{
  tone(SPEAKER_A, beep_tone, beep_ms);
}

void SendBrightness(unsigned int brightness)
{
  byte temp_brightness[4] = {};
  temp_brightness[0] = (brightness & 0x000000FF);
  temp_brightness[1] = (brightness & 0x0000FF00) >> 8;
  temp_brightness[2] = (brightness & 0x00FF0000) >> 16;
  temp_brightness[3] = (brightness & 0xFF000000) >> 24;

  Wire.beginTransmission(0x33);  //Begin transmission
  for (int i = 0; i < 4; i++)
  {
    Wire.write(temp_brightness[i]);
  }
  Wire.endTransmission();  //Stop transmission

  //Beep completion
  Beep(NOTE_G3, 100);
}

/*
* My light control
* Date: September 16 2014
* Written by Adrien
*/

#include <Wire.h>  //I2C Slave
#define I2C_SL_ADDR 0x33

#define  NUM_TLCS  1  //TLC units
#define  NUM_ROWS  1  //Number of row per TLC
#include "Tlc5940Mux.h"

const int TLC_GSCLK = 3;
const int TLC_XLAT = 9;
const int TLC_BLANK = 10;
const int TLC_SIN = 12;
const int TLC_SCLK = 13;

const int LED_H1 = 4;
const int LED_H2 = 7;
const int LED_H3 = 8;

//TLC ISR
volatile uint8_t isShifting;
uint8_t shiftRow;

ISR(TIMER1_OVF_vect)
{
  if (!isShifting)
  {
    disable_XLAT_pulses();
    isShifting = 1;
    sei();
    TlcMux_shiftRow(shiftRow);
    PORTC = shiftRow++;
    if (shiftRow == NUM_ROWS)
    {
      shiftRow = 0;
    }
    enable_XLAT_pulses();
    isShifting = 0;
  }
}

//Arduino setup
void setup()
{
  //Begin I2C as Slave with address 0x33
  Wire.begin(I2C_SL_ADDR);
  Wire.onReceive(receiveEvent);

  //Begin serial
  Serial.begin(9600);

  //Set digital IO
  pinMode(LED_H1, OUTPUT);      // sets the digital pin as output
  pinMode(LED_H2, OUTPUT);      // sets the digital pin as output
  pinMode(LED_H3, OUTPUT);      // sets the digital pin as output

  //Set TLC output
  pinMode(TLC_GSCLK, OUTPUT);  // sets the digital pin as output
  pinMode(TLC_XLAT, OUTPUT);   // sets the digital pin as output
  pinMode(TLC_BLANK, OUTPUT);  // sets the digital pin as output
  pinMode(TLC_SIN, OUTPUT);    // sets the digital pin as output
  pinMode(TLC_SCLK, OUTPUT);   // sets the digital pin as output

  //Set IO
  set_h(0);

  //TLC setup
  DDRC |= _BV(PC0) | _BV(PC1) | _BV(PC2);  //Override DDRC
  TlcMux_init();  //Initialize TLC
}

int _myrow = 0;
volatile unsigned int _ledbrightness = 0;
volatile bool changeOnce = false;
//Arduino loop
void loop()
{
  if (_ledbrightness > 0)
  {
    if (changeOnce == true)
    {
      TlcMux_clear();  //Clear TLC
      delay(1);  //Avoid flicker

      //Do something
      //TlcMux_clear();  //Clear TLC
      for (uint8_t col = 0; col < 16; col++)
      {
        for (uint8_t row = 0; row < NUM_ROWS; row++)
        {
          TlcMux_set(row, col, _ledbrightness);
        }
        //delay(30);
      }
      changeOnce = false;  //Updated, clear flag
    }

    //Change row
    if (_myrow > 3) _myrow = 1;
    set_h(_myrow);
    _myrow++;
  }
  else
  {
    delay(500);
  }
}

void set_h(int i)
{
  //TlcMux_clear();  //Clear TLC before swiching lines
  switch (i)
  {
    case 1:
      //Serial.println("HIGH_1");
      digitalWrite(LED_H1, HIGH);
      digitalWrite(LED_H2, LOW);
      digitalWrite(LED_H3, LOW);
      break;

    case 2:
      //Serial.println("HIGH_2");
      digitalWrite(LED_H1, LOW);
      digitalWrite(LED_H2, HIGH);
      digitalWrite(LED_H3, LOW);
      break;

    case 3:
      //Serial.println("HIGH_3");
      digitalWrite(LED_H1, LOW);
      digitalWrite(LED_H2, LOW);
      digitalWrite(LED_H3, HIGH);
      break;

    case 0:
    default:
      Serial.println("HIGH_OFF");
      digitalWrite(LED_H1, LOW);
      digitalWrite(LED_H2, LOW);
      digitalWrite(LED_H3, LOW);
      break;
  }
}

//As slave
void receiveEvent(int nbytes)
{
  if (nbytes > 4) return;  //Invalid quick check

  int temp_brightness = 0;

  byte temp_bytes[4] = {};

  for (int i = 0; i < nbytes; i++)
  {
    temp_bytes[i] = Wire.read();
  }

  temp_brightness += ((int)temp_bytes[0]);
  temp_brightness += ((int)temp_bytes[1]) << 8;
  temp_brightness += ((int)temp_bytes[2]) << 16;
  temp_brightness += ((int)temp_bytes[3]) << 24;

  _ledbrightness = temp_brightness;

  //Automatic clear
  if (_ledbrightness == 0)
  {
    TlcMux_clear();  //Clear TLC
    set_h(0);
  }

  //Flag to change value once
  changeOnce = true;
}

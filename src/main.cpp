#include <Arduino.h>
#include "GyverSegment.h"

#define PERIOD 200
#define PERIODS 1000
uint32_t timer = 0;
uint32_t timer2 = 0;

uint8_t count = 0;

char fw[4] = {"0.2"};

uint8_t digs[] = {D0, D1}; // пины цифр
// uint8_t segs[] = {D2, D3, D4, A0, D5, D6, D7, D8}; // пины сегментов
// 0bPGFEDCBA
// uint8_t segs[] = {D8, D7, D6, D5, A0, D4, D3, D2}; // пины сегментов
uint8_t segs[] = {D8, D7, D6, D5, D2, D4, D3, A0}; // пины сегментов
// 8 цифр, дсятичные точки нет, общий катод
DispBare<2, false, false> disp(digs, segs);

// Prepare binary array for all 7 segment to turn on 7 segment at position of a,b,c,d,e,f,g
int digit[10] = {0b0111111, 0b0000110, 0b1011011, 0b1001111, 0b1100110, 0b1101101, 0b1111101, 0b0000111, 0b1111111, 0b1101111};
int digit1, digit2; // initialize individual digit to controll each segment

#define SPEED 115200

void setup()
{
  Serial.begin(SPEED);
  Serial.println("GPS Speed Controller");
  for (uint8 i = 0; i <= 4; i++)
  {
    disp.setCursorEnd();
    disp.printRight(true);
    disp.fillChar('-');
    disp.update();
    disp.delay(500);
    disp.clear();
    disp.update();
    disp.delay(500);
  }
}

void loop()
{
  // delay(200);
  if (millis() - timer >= 10)
  {
    // ваше действие
    disp.setCursorEnd();
    disp.printRight(true);
    disp.fillChar('0');
    // disp.setCursor(1);
    disp.print(count);
    disp.update();
    disp.tick();

    // disp.delay(10);
    timer += 10;
  }

  if (millis() - timer2 >= 1000)
  {
    count < 99 ? count++ : count = 0;
    timer2 += 100;
  }
}

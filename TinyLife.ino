/*************************************************** 
  This is a library for our I2C LED Backpacks

  Designed specifically to work with the Adafruit LED Matrix backpacks 
  ----> http://www.adafruit.com/products/872
  ----> http://www.adafruit.com/products/871
  ----> http://www.adafruit.com/products/870

  These displays use I2C to communicate, 2 pins are required to 
  interface. There are multiple selectable I2C addresses. For backpacks
  with 2 Address Select pins: 0x70, 0x71, 0x72 or 0x73. For backpacks
  with 3 Address Select pins: 0x70 thru 0x77

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <USI_TWI_Master.h>
#include <TinyWireM.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

uint8_t liveOrDie(uint8_t row, uint8_t col, uint8_t * cells) {
  uint8_t above;
  uint8_t below;
  uint8_t right;
  uint8_t left;
  uint8_t count;

  count = 0;

  above = row - 1;
  if (above > 7) { above = 7; }

  below = row + 1;
  if (below > 7) { below = 0; }

  right = col >> 1;
  if (right == 0) { right = 0x80; }

  left = col << 1;
  if (left == 0) { left = 0x01; }

  if (cells[row] & right)   { count = count + 1; }
  if (cells[row] & left)    { count = count + 1; }

  if (cells[above] & col)   { count = count + 1; }
  if (cells[above] & right) { count = count + 1; }
  if (cells[above] & left)  { count = count + 1; }

  if (cells[below] & col)   { count = count + 1; }
  if (cells[below] & right) { count = count + 1; }
  if (cells[below] & left)  { count = count + 1; }

  if (cells[row] & col) {
     if (count < 2) return 0;
     if (count > 3) return 0;
  } else {
    if (count == 3) return col;
  }

  return cells[row] & col;
}

void repopulate(uint8_t * cells, uint8_t * newCells) {
   uint8_t row;
   uint8_t col;

   for (row = 0; row < 8; row++) {
      col = 0x01;
      while (col != 0) {
         if (liveOrDie(row, col, cells)) {
            newCells[row] = newCells[row] | col;
         } else {
            newCells[row] = newCells[row] & ~col;
         }
         col = col << 1;
      }
   }
}

Adafruit_BicolorMatrix matrix = Adafruit_BicolorMatrix();

uint8_t petri_dish[3][8];
uint8_t current;
uint8_t previous;
uint8_t really_old;


void matrix_update(uint8_t * values, uint8_t color) {
  int i, j;
  
  for (i = 0; i < 8; i++) {
    for (j = 0; j < 8; j++) {
      if (values[i] & (1 << j)) {
        matrix.drawPixel(i, j, color);
      }
    }
  }
}

void setup() {
  int i;

  randomSeed(analogRead(0));

  current = 0;
  previous = 2;
  really_old = 1;
  
  for (i = 0; i < 8; i++) {
    petri_dish[current][i] = random(256);
    //petri_dish[current][i] = 0;
    petri_dish[previous][i] = 0;
    petri_dish[really_old][i] = 0;
  }
  
  //petri_dish[current][1] = 0x02;
  //petri_dish[current][2] = 0x04;
  //petri_dish[current][3] = 0x07;
  matrix.begin(0x70);  // pass in the address
  
  matrix.clear();
  matrix_update(petri_dish[current], LED_GREEN);
  matrix.writeDisplay();
  delay(1000);
}

void loop() {
  uint8_t next;
  uint8_t i;

  matrix.clear();
  for (i = 0; i < 8; i++) {
    petri_dish[really_old][i] &= ~(petri_dish[previous][i] | petri_dish[current][i]);
    petri_dish[previous][i] &= ~(petri_dish[current][i]);
  }
  matrix_update(petri_dish[really_old], LED_RED);
  matrix_update(petri_dish[previous], LED_YELLOW);
  matrix_update(petri_dish[current], LED_GREEN);
  matrix.writeDisplay();
  delay(500);

  next = (current + 1) % 3;
  repopulate(petri_dish[current], petri_dish[next]);

  really_old = previous;
  previous = current;
  current = next;
}

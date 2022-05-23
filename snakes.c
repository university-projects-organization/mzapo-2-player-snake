/*******************************************************************
  Project main function template for MicroZed based MZ_APO board
  designed by Petr Porazil at PiKRON

  change_me.c      - main file

  include your name there and license for distribution.

  Remove next text: This line should not appear in submitted
  work and project name should be change to match real application.
  If this text is there I want 10 points subtracted from final
  evaluation.

 *******************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

#include "mzapo_parlcd.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
#include "serialize_lock.h"
#include "font_types.h"

union led {
  struct {
    uint8_t r, g, b;
  };
  uint32_t data;
};

union pixel {
  struct {
    unsigned b : 5;
    unsigned g : 6;
    unsigned r : 5;
  };
  uint16_t d;
};

union pixel buffer[480][320];
/*
font_descriptor_t *font = &font_rom8x16;

void pchar(char c, unsigned x, unsigned y) {
  for (unsigned w = 0; w < font->maxwidth; w++) {
    for (unsigned h = 0; h < font->height; h++) {
      if ((font->bits[(c - font->firstchar) * font->height + h] & (1<<w))) {
        buffer[x + w][y + h] = (union pixel){ .b = 0x1f};
      }
    }
  }
}
*/

int main(int argc, char *argv[])
{
  volatile void *spiled_reg_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
  volatile uint32_t *ledline = (spiled_reg_base + SPILED_REG_LED_LINE_o);
  volatile uint32_t *rgb1 = (spiled_reg_base + SPILED_REG_LED_RGB1_o);
  *ledline =0x6600;
  *rgb1 = ((union led){ .r = 0xff, .g = 0xff }).data;

  for(unsigned x = 0; x < 480; x++) {
    for(unsigned y = 0; y < 320; y++) {
      buffer[x][y].d = 0xcffc;
    }
  }

  unsigned char *parlcd_reg_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
  int16_t xx = -100;
  int t = 1000;

  while(t > 0) {
    t--;
    
    if (xx >= 480) {
      xx = -100;
    }

    for(unsigned x = 0; x < 480; x++) {
      for(unsigned y = 0; y < 320; y++) {
        buffer[x][y].d = 0xcffc;
      }
    }

    for(unsigned yy = 0; yy < 100; yy++) {
      if (xx < 480 && xx >= 0) buffer[xx][yy + 20] = (union pixel){ .b = 0x1f}; // left 
      if (xx + yy < 480 && xx + yy >= 0) buffer[xx + yy][20] = (union pixel){ .b = 0x1f}; // top
      if (xx + 100 < 480 && xx + 100 >= 0) buffer[xx + 100][yy + 20] = (union pixel){ .b = 0x1f}; // right
      if (xx + yy < 480 && xx + yy >= 0) buffer[xx + yy][120] = (union pixel){ .b = 0x1f}; // bottom

      if (xx < 480 && xx >= 0) buffer[xx][yy + 140] = (union pixel){ .b = 0x1f}; // left 
      if (xx + yy < 480 && xx + yy >= 0) buffer[xx + yy][140] = (union pixel){ .b = 0x1f}; // top
      if (xx + 100 < 480 && xx + 100 >= 0) buffer[xx + 100][yy + 140] = (union pixel){ .b = 0x1f}; // right
      if (xx + yy < 480 && xx + yy >= 0) buffer[xx + yy][240] = (union pixel){ .b = 0x1f}; // bottom
    }
    xx+=8;

    parlcd_write_cmd(parlcd_reg_base, 0x2c);
    for(unsigned y = 0; y < 320; y++) {
      for(unsigned x = 0; x < 480; x++) {
        parlcd_write_data(parlcd_reg_base, buffer[x][y].d);
      }
    }
  }
  return 0;
}

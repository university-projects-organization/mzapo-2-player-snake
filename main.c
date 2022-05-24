/*******************************************************************
  Nazar Ponomarev, Nikita Kisel
 *******************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "mzapo_parlcd.h"
#include "game_menu.h"
#include "mzapo_phys.h"
#include "mzapo_regs.h"
#include "pixel_structure.h"
#include "screen.h"
#include "stb_library/stb_image.h"
#include "stb_library/stb_image_write.h"
#include "game.h"

int main(int argc, char *argv[]) {
    volatile void *spiled_reg_base = map_phys_address(SPILED_REG_BASE_PHYS, SPILED_REG_SIZE, 0);
    unsigned char *parlcd_reg_base = map_phys_address(PARLCD_REG_BASE_PHYS, PARLCD_REG_SIZE, 0);
    parlcd_write_cmd(parlcd_reg_base, 0x2c);

    uint8_t *settings = (uint8_t *) malloc(sizeof(uint8_t) * 7);
    for (size_t i = 0; i < 7; i++) {
        settings[i] = 0;
    }

    uint32_t colorLed1 = (255 << 16) + (255 << 8) + 150;
    uint32_t colorLed2 = (255 << 16) + (255 << 8) + 150;

    union pixel **screen = allocateScreen();
    if (!gameMenu(screen, settings, spiled_reg_base, parlcd_reg_base, &colorLed1, &colorLed2)) {
        return 0;
    }
    game(settings, spiled_reg_base, parlcd_reg_base, screen, &colorLed1, &colorLed2);
}
#ifndef _SCREEN_H
#define _SCREEN_H

#define MENU_HEIGHT (50)
#define MENU_WIDTH (50)
// so menu height x 2 == meter height (but we still have 20 pixels
// empty.
#define METER_HEIGHT (100) // edit default = 60
#define METER_WIDTH (240)
#define PANADAPTER_HEIGHT (105)
#define ZOOMPAN_HEIGHT (50)
#define SLIDERS_HEIGHT (100)
#define TOOLBAR_HEIGHT (30)
#define WATERFALL_HEIGHT (105)

// define a type and coordinates for various widgets on the screen.
#define  VFO_HEIGHT (100)
#define  VFO_WIDTH  (display_width - METER_WIDTH - MENU_WIDTH)

#define  BLACK_R (0.0)
#define  BLACK_G (0.0)
#define  BLACK_B (0.0)

#define  RED_R (1.0)
#define  RED_G (0.0)
#define  RED_B (0.0)

#define  YELLOW_R (1.0)
#define  YELLOW_G (1.0)
#define  YELLOW_B (0.0)

#define  CYAN_R (0.0)
#define  CYAN_G (1.0)
#define  CYAN_B (1.0)

#define  WHITE_R (1.0)
#define  WHITE_G (1.0)
#define  WHITE_B (1.0)

#define  GREEN_R (0.0)
#define  GREEN_G (1.0)
#define  GREEN_B (0.0)

#define  DARK_GREEN_R (0.0)
#define  DARK_GREEN_G (0.65)
#define  DARK_GREEN_B (0.0)


// font/typeface sizes

#define  MODE_RENDER_FONT_SIZE   (30)
#define  VFO_A_RENDER_FONT_SIZE  (70)

// coordinates

#define  MODE_X     70
#define  MODE_Y     80

#define  VFO_A_X    285
#define  VFO_A_Y    95

#endif // _SCREEN_H
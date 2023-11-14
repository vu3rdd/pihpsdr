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

#define  BLACK_R  (0.0)
#define  BLACK_G  (0.0)
#define  BLACK_B  (0.0)

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

#define  GREY_R (0.7)
#define  GREY_G (0.7)
#define  GREY_B (0.7)

// font/typeface sizes

#define  MODE_RENDER_FONT_SIZE      (30)
#define  VFO_A_RENDER_FONT_SIZE     (70)
#define  VFO_B_RENDER_FONT_SIZE     (18)
#define  ACTIVE_VFO_INDICATION_SIZE (18)
#define  PS_RENDER_FONT_SIZE        (12)
#define  RIT_RENDER_FONT_SIZE       (16)
#define  XIT_RENDER_FONT_SIZE       (16)
#define  NB_RENDER_FONT_SIZE        (16)
#define  NR_RENDER_FONT_SIZE        (16)
#define  DUP_RENDER_FONT_SIZE       (16)
#define  SPLIT_RENDER_FONT_SIZE     (16)
// coordinates

#define  MODE_X     70
#define  MODE_Y     80

#define  VFO_A_X    285
#define  VFO_A_Y    95

#define  VFO_B_X    285
#define  VFO_B_Y    18

#define  ACTIVE_VFO_INDICATION_X  400
#define  ACTIVE_VFO_INDICATION_Y   20

#define  PS_X       130
#define  PS_Y       50

#define  RIT_X      220
#define  RIT_Y      40

#define  XIT_X      220
#define  XIT_Y      20

#define  NB_X       115
#define  NB_Y       40

#define  NR_X       70
#define  NR_Y       40

#define  ANF_X      70
#define  ANF_Y      20

#define  SNB_X      115
#define  SNB_Y      20

#define  AGC_X      160
#define  AGC_Y      20

#define  MIDI_X     480
#define  MIDI_Y     20

#define  VOX_X      160
#define  VOX_Y      40

#define  LOCK_X      5
#define  LOCK_Y      80

#define  SPLIT_X     5
#define  SPLIT_Y     20

#define  CTUN_X      5
#define  CTUN_Y      40

#define  DUP_X       5
#define  DUP_Y       60

#endif // _SCREEN_H

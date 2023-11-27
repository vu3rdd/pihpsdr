#ifndef _SCREEN_H
#define _SCREEN_H

#include <stddef.h>

#define MENU_HEIGHT (50)
#define MENU_WIDTH (50)

// menu height x 2 == meter height (but we still have 20 pixels
// empty.
#define METER_HEIGHT (100) // edit default = 60
#define METER_WIDTH (240)
#define PANADAPTER_HEIGHT (105)
#define ZOOMPAN_HEIGHT (50)
#define SLIDERS_HEIGHT (100)
#define TOOLBAR_HEIGHT (30)
#define WATERFALL_HEIGHT (105)

typedef struct colour {
    float r;
    float g;
    float b;
} colour_t;

typedef struct widget_props {
    size_t x;
    size_t y;
    size_t font_size;
    colour_t colours[5];
    char *label[5];
} widget_props_t;

enum on_screen_buttons {
    SCR_VFO_A = 0,
    SCR_VFO_B,
    SCR_ACTIVE_VFO,
    SCR_MODE,
    SCR_PS,
    SCR_RIT,
    SCR_XIT,
    SCR_NB,
    SCR_NR,
    SCR_ANF,
    SCR_SNB,
    SCR_MIDI,
    SCR_AGC,
    SCR_VOX,
    SCR_LOCK,
    SCR_CTUN,
    SCR_SPLIT,
    SCR_DUP,
    NUM_ACTIVE_BUTTONS,
};

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

extern widget_props_t default_widget_prop_table[];

#endif // _SCREEN_H

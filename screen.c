#include "screen.h"

const colour_t grey = { GREY_R, GREY_G, GREY_B };
const colour_t red = { RED_R, RED_G, RED_B};
const colour_t black = { BLACK_R, BLACK_G, BLACK_B };
const colour_t white = { WHITE_R, WHITE_G, WHITE_B };
const colour_t yellow = { YELLOW_R, YELLOW_G, YELLOW_B };
const colour_t cyan = { CYAN_R, CYAN_G, CYAN_B };
const colour_t green = { GREEN_R, GREEN_G, GREEN_B };
const colour_t dark_green = { DARK_GREEN_R, DARK_GREEN_G, DARK_GREEN_B };

widget_props_t default_widget_prop_table[NUM_ACTIVE_BUTTONS] = {
    // label, x, y, font_size, off_colour, on_colour
    [SCR_VFO_A] = { "", 285, 95, 70, white, green },
    [SCR_VFO_B] = { "", 285, 18, 18, grey, cyan },
    [SCR_ACTIVE_VFO] = { "", 400, 20, 18, yellow, yellow },
    [SCR_MODE] = { "MODE", 70, 80, 30, yellow, yellow },
    [SCR_PS] = { "PS", 120, 50, 12, yellow, grey },
    [SCR_RIT] = { "RIT", 220, 40, 16, red, grey },
    [SCR_XIT] = { "XIT", 220, 20, 16, red, grey },
    [SCR_NB] = { "NB", 115, 40, 16, yellow, grey },
    [SCR_NR] = { "NR", 70, 40, 16, yellow, grey },
    [SCR_ANF] = { "ANF", 70, 20, 16, yellow, grey },
    [SCR_SNB] = { "SNB", 115, 20, 16, yellow, grey },
    [SCR_MIDI] = { "MIDI", 480, 20, 16, yellow, grey },
    [SCR_AGC] = { "AGC", 160, 20, 16, yellow, grey },
    [SCR_VOX] = { "VOX", 160, 40, 16, red, grey },
    [SCR_LOCK] = { "LOCK", 5, 80, 16, red, grey },
    [SCR_CTUN] = { "CTUN", 5, 40, 16, yellow, grey },
    [SCR_SPLIT] = { "SPLIT", 5, 20, 16, red, grey, },
    [SCR_DUP] = {"DUP", 5, 60, 16, red, grey },
};

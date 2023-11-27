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
    [SCR_VFO_A] = { 285, 95, 70, { green, white }, { "", "" } },
    [SCR_VFO_B] = { 285, 18, 18, { cyan, grey}, { "", "" } },
    [SCR_ACTIVE_VFO] = { 400, 20, 18, { yellow, yellow }, { "", "" } },
    [SCR_MODE] = { 70, 80, 30, { yellow, yellow }, { "MODE", "MODE" } },
    [SCR_PS] = { 120, 50, 12, { grey, yellow }, { "PS", "PS" } },
    [SCR_RIT] = { 220, 40, 16, { grey, red }, { "RIT", "RIT" } },
    [SCR_XIT] = { 220, 20, 16, { grey, red }, { "XIT", "XIT" } },
    [SCR_NB] = { 115, 40, 16, { grey, yellow }, { "NB", "NB", "NB2" } },
    [SCR_NR] = { 70, 40, 16, { grey, yellow, yellow, yellow, yellow }, { "NR", "NR", "NR2", "NR3", "NR4" } },
    [SCR_ANF] = { 70, 20, 16, { grey, yellow }, { "ANF", "ANF" } },
    [SCR_SNB] = { 115, 20, 16, { grey, yellow }, { "SNB", "SNB" } },
    [SCR_MIDI] = { 480, 20, 16, { grey, yellow }, { "MIDI", "MIDI" } },
    [SCR_AGC] = { 160, 20, 16, { grey, yellow, yellow, yellow, yellow }, { "AGC", "AGC L", "AGC S", "AGC M", "AGC F" } },
    [SCR_VOX] = { 160, 40, 16, { grey, red }, { "VOX", "VOX" } },
    [SCR_LOCK] = { 5, 80, 16, { grey, red }, { "LOCK", "LOCK" } },
    [SCR_CTUN] = { 5, 40, 16, { grey, yellow }, { "CTUN", "CTUN" } },
    [SCR_SPLIT] = { 5, 20, 16, { grey, red }, { "SPLIT", "SPLIT" } },
    [SCR_DUP] = { 5, 60, 16, { grey, red }, { "DUP", "DUP" } },
};

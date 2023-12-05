#ifndef RIGCTL_H
#define RIGCTL_H

#include <glib.h>     // for gpointer
#include <stdbool.h>  // for bool
#include <stdint.h>   // for uint32_t

void launch_rigctl ();
int launch_serial ();
void disable_sreial ();

void  close_rigctl_ports ();
int   rigctlGetMode();
int   lookup_band(int);
char * rigctlGetFilter();
void set_freqB(long long);
extern int cat_control;
int set_alc(gpointer);
extern int rigctl_busy;

extern int rigctl_port_base;
extern int rigctl_enable;

typedef enum cmd_arg_t {
    cmd_type_none = 0,
    cmd_type_string,
    cmd_type_bool,
    cmd_type_num,
} cmd_arg_t;

typedef struct cat_command {
    char      *cmd;
    cmd_arg_t type;
    bool      is_implemented;
    uint32_t  arg_len; // in bytes
    bool      with_sign;  // true if there sign is part of the argument
    int       min_arg_value;
    int       max_arg_value;
} cat_command;

enum cmds {
    // #S
    CMD_AC = 1,
    CMD_AG,
    CMD_AI,
    CMD_AL,
    CMD_AM,
    CMD_AN,
    CMD_AS,
    CMD_BC,
    CMD_BD,
    CMD_BP,
    CMD_BS,
    CMD_BU,
    CMD_BY,
    CMD_CA,
    CMD_CG,
    CMD_CI,
    CMD_CM,
    CMD_CN,
    CMD_CT,
    CMD_DC,
    CMD_DN,
    CMD_DQ,
    CMD_EX,
    CMD_FA,
    CMD_FB,
    CMD_FC,
    CMD_FD,
    CMD_FR,
    CMD_FS,
    CMD_FT,
    CMD_GT,
    CMD_ID,
    CMD_IF,
    CMD_IS,
    CMD_KS,
    CMD_KY,
    CMD_LK,
    CMD_LM,
    CMD_LT,
    CMD_MC,
    CMD_MD,
    CMD_MF,
    CMD_MG,
    CMD_ML,
    CMD_MO,
    CMD_MR,
    CMD_MU,
    CMD_MW,
    CMD_NB,
    CMD_NL,
    CMD_NR,
    CMD_NT,
    CMD_OF,
    CMD_OI,
    CMD_OS,
    CMD_PA,
    CMD_PB,
    CMD_PC,
    CMD_PI,
    CMD_PK,
    CMD_PL,
    CMD_PM,
    CMD_PR,
    CMD_PS,
    CMD_QC,
    CMD_QI,
    CMD_QR,
    CMD_RA,
    CMD_RC,
    CMD_RD,
    CMD_RG,
    CMD_RL,
    CMD_RM,
    CMD_RT,
    CMD_RU,
    CMD_RX,
    CMD_SA,
    CMD_SB,
    CMD_SC,
    CMD_SD,
    CMD_SH,
    CMD_SI,
    CMD_SL,
    CMD_SM,
    CMD_SQ,
    CMD_SR,
    CMD_SS,
    CMD_ST,
    CMD_SU,
    CMD_SV,
    CMD_TC,
    CMD_TD,
    CMD_TI,
    CMD_TN,
    CMD_TO,
    CMD_TS,
    CMD_TX,
    CMD_TY,
    CMD_UL,
    CMD_UP,
    CMD_VD,
    CMD_VG,
    CMD_VR,
    CMD_VX,
    CMD_XT,
    NUM_CMDS, // define new commands above this line
};



#endif // RIGCTL_H

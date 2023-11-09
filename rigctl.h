#ifndef RIGCTL_H
#define RIGCTL_H

#include <stdbool.h>

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
    AC = 1,
    AG,
    AI,
    AL,
    AM,
    AN,
    AS,
    BC,
    BD,
    BP,
    BS,
    BU,
    BY,
    CA,
    CG,
    CI,
    CM,
    CN,
    CT,
    DC,
    DN,
    DQ,
    EX,
    FA,
    FB,
    FC,
    FD,
    FR,
    FS,
    FT,
    GT,
    ID,
    IF,
    IS,
    KS,
    KY,
    LK,
    LM,
    LT,
    MC,
    MD,
    MF,
    MG,
    ML,
    MO,
    MR,
    MU,
    MW,
    NB,
    NL,
    NR,
    NT,
    OF,
    OI,
    OS,
    PA,
    PB,
    PC,
    PI,
    PK,
    PL,
    PM,
    PR,
    PS,
    QC,
    QI,
    QR,
    RA,
    RC,
    RD,
    RG,
    RL,
    RM,
    RT,
    RU,
    RX,
    SA,
    SB,
    SC,
    SD,
    SH,
    SI,
    SL,
    SM,
    SQ,
    SR,
    SS,
    ST,
    SU,
    SV,
    TC,
    TD,
    TI,
    TN,
    TO,
    TS,
    TX,
    TY,
    UL,
    UP,
    VD,
    VG,
    VR,
    VX,
    XT,
    NUM_CMDS, // define new commands above this line
};



#endif // RIGCTL_H

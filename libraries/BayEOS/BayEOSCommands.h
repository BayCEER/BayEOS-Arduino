/*
 * BayEOS-Commands
 *
 *
 */

// SwitchCommands: [0x2][0x30][command][channel - 0=ALL][args]
#define BayEOS_SwitchCommand 0x30
#define SWITCH_ON 0x1
#define SWITCH_OFF 0x2
#define SWITCH_SLOW_ON 0x3
#define SWITCH_SLOW_OFF 0x4
#define SWITCH_POWER_PLUS 0x5
#define SWITCH_POWER_MINUS 0x6
#define SWITCH_POWER_SET 0x7
#define SWITCH_ONOFF 0x8
#define SWITCH_SLOW_ONOFF 0x9
#define SWITCH_ON_WITHTIMEOUT 0xa
#define SWITCH_RESET_KWH 0xb

// Router Commands: [0x2][0x31][command]
#define BayEOS_RouterCommand 0x31
#define ROUTER_IS_READY 0x1
// Returns [0x3][0x31][command][uint8_t]
#define ROUTER_GET_AVAILABLE 0x2
// Returns [0x3][0x31][command][unsigned long]
#define ROUTER_SEND 0x3
// Returns [0x3][0x31][command][uint8_t]






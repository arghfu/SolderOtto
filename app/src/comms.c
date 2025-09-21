#include "comms.h"

K_MSGQ_DEFINE(control_msgq, sizeof(struct display_msg), 10, 4);
/**
 * This file should always be included instead of <rfb/rfb.h> because otherwise it's redefinition
 * of TRUE and FALSE plays havoc with other things.
 */

#ifndef KRFB_RFB_H
#define KRFB_RFB_H

#include "../libvncserver/rfb/rfb.h"

#undef TRUE
#undef FALSE

#endif // Header guard


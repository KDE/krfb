/**
 * This file should always be included instead of <rfb/rfb.h> because otherwise it's redefinition
 * of TRUE and FALSE plays havoc with other things.
 */

#ifndef KRFB_RFB_H
#define KRFB_RFB_H

#include "rfb/rfb.h"

#ifdef max
#undef max
#endif

#undef TRUE
#undef FALSE

#endif // Header guard

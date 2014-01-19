# Try to find the KTp library
# KTP_FOUND
# KTP_INCLUDE_DIR
# KTP_LIBRARIES
# KTP_MODELS_LIBRARIES
# KTP_WIDGETS_LIBRARIES

# Copyright (c) 2011, Dario Freddi <drf@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (NOT IS_KTP_INTERNAL_MODULE)
   message (FATAL_ERROR "KTp can be used just from internal components at this time")
endif (NOT IS_KTP_INTERNAL_MODULE)

SET (KTP_FIND_REQUIRED ${KTp_FIND_REQUIRED})
if (KTP_INCLUDE_DIRS AND KTP_LIBRARIES)
  # Already in cache, be silent
  set(KTP_FIND_QUIETLY TRUE)
endif (KTP_INCLUDE_DIRS AND KTP_LIBRARIES)

find_path(KTP_INCLUDE_DIR
  NAMES KTp/presence.h
  PATHS ${KDE4_INCLUDE_DIR}
)

find_library(KTP_LIBRARIES NAMES ktpcommoninternalsprivate )
find_library(KTP_MODELS_LIBRARIES NAMES ktpmodelsprivate )
find_library(KTP_WIDGETS_LIBRARIES NAMES ktpwidgetsprivate )

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(KTp DEFAULT_MSG
                                  KTP_LIBRARIES
                                  KTP_MODELS_LIBRARIES
                                  KTP_INCLUDE_DIR)

mark_as_advanced(KTP_INCLUDE_DIRS KTP_LIBRARIES)

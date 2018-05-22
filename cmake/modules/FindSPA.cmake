#.rst:
# FindSPA
# -------
#
# Try to find the Simple Plugin API (SPA) on a Unix system.
#
# This will define the following variables:
#
# ``SPA_FOUND``
#     True if (the requested version of) SPA is available
# ``SPA_VERSION``
#     The version of SPA
# ``SPA_LIBRARIES``
#     This can be passed to target_link_libraries() instead of the ``SPA::SPA``
#     target
# ``SPA_INCLUDE_DIRSS``
#     This should be passed to target_include_directories() if the target is not
#     used for linking
# ``SPA_DEFINITIONS``
#     This should be passed to target_compile_options() if the target is not
#     used for linking
#
# If ``SPA_FOUND`` is TRUE, it will also define the following imported target:
#
# ``SPA::SPA``
#     The SPA library
#
# In general we recommend using the imported target, as it is easier to use.
# Bear in mind, however, that if the target is in the link interface of an
# exported library, it must be made available by the package config file.

#=============================================================================
# Copyright 2014 Alex Merry <alex.merry@kde.org>
# Copyright 2014 Martin Gräßlin <mgraesslin@kde.org>
# Copyright 2018 Jan Grulich <jgrulich@redhat.com>

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#=============================================================================


# Use pkg-config to get the directories and then use these values
# in the FIND_PATH() and FIND_LIBRARY() calls
find_package(PkgConfig QUIET)
pkg_check_modules(PKG_SPA QUIET libspa-0.1)

set(SPA_DEFINITIONS "${PKG_SPA_CFLAGS_OTHER}")
set(SPA_VERSION "${PKG_SPA_VERSION}")

find_path(SPA_INCLUDE_DIRS
    NAMES
        spa/pod/pod.h
    HINTS
        ${PKG_SPA_INCLUDE_DIRS}
)

find_library(SPA_LIBRARIES
    NAMES
        spa-lib
    HINTS
        ${PKG_SPA_LIBRARIES_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SPA
    FOUND_VAR
        SPA_FOUND
    REQUIRED_VARS
        SPA_LIBRARIES
        SPA_INCLUDE_DIRS
    VERSION_VAR
        SPA_VERSION
)

if(SPA_FOUND AND NOT TARGET SPA::SPA)
    add_library(SPA::SPA UNKNOWN IMPORTED)
    set_target_properties(SPA::SPA PROPERTIES
        IMPORTED_LOCATION "${SPA_LIBRARIES}"
        INTERFACE_COMPILE_OPTIONS "${SPA_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${SPA_INCLUDE_DIRS}"
    )
endif()

mark_as_advanced(SPA_LIBRARIES SPA_INCLUDE_DIRS)

include(FeatureSummary)
set_package_properties(SPA PROPERTIES
    DESCRIPTION "Simple Plugin API"
)

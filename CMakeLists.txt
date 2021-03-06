project(kdepim)

# where to look first for cmake modules. This line must be the first one or cmake will use the system's FindFoo.cmake
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")


############### Build Options ###############

option(KDEPIM_BUILD_EXAMPLES "Build the kdepim example applications." FALSE)
option(KDEPIM_ENTERPRISE_BUILD "Enable features specific to the enterprise branch, which are normally disabled. Also, it disables many components not needed for Kontact such as the Kolab client." FALSE)
option(KDEPIM_ONLY_KLEO "Only build Kleopatra. This option will disable KDEPIM_BUILD_EVERYTHING and KDEPIM_ENTERPRISE_BUILD." FALSE)

# if KDEPIM_ENTERPRISE_BUILD is defined, KDEPIM_ONLY_KLEO is disabled
if(KDEPIM_ENTERPRISE_BUILD)
  set(KDEPIM_ONLY_KLEO FALSE)
  set(KDEPIM_BUILD_EVERYTHING FALSE)
  message(STATUS "Enterprise build is enabled.")
else(KDEPIM_ENTERPRISE_BUILD)
  # if KDEPIM_ONLY_KLEO is defined, KDEPIM_BUILD_EVERYTHING is disabled.
  if(KDEPIM_ONLY_KLEO)
    set(KDEPIM_BUILD_EVERYTHING FALSE)
    set(KDEPIM_DEFINITIONS "-DHAVE_CONFIG_H=1")
    message(STATUS "Only libkleo and Kleopatra will be built.")
  endif(KDEPIM_ONLY_KLEO)
  # Default
    set(KDEPIM_BUILD_EVERYTHING TRUE)
    message(STATUS "Enterprise build is disabled.")
endif(KDEPIM_ENTERPRISE_BUILD)

# config-enterprise.h is needed for both ENTERPRISE_BUILD and BUILD_EVERYTHING
configure_file(config-enterprise.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-enterprise.h )


############### search packages used by KDE ###############

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include(CheckIncludeFiles)
include(MacroBoolTo01)
find_package(KdepimLibs 4.3.85 REQUIRED)


############### search Boost ###############

find_package(Boost 1.34.0)
macro_log_feature(Boost_FOUND "Boost" "Boost C++ Libraries" "http://www.boost.org" TRUE "1.34.0" "Required by several critical KDEPIM apps.")

# Kleopatra needs to know if the topological.hpp header exists (part of Boost_graph).
find_path(Boost_TOPOLOGICAL_SORT_DIR NAMES boost/graph/topological_sort.hpp PATHS ${Boost_INCLUDE_DIRS})
macro_log_feature(Boost_TOPOLOGICAL_SORT_DIR "The Boost Topological_sort header" "topological_sort.hpp (part of Boost_graph)" "http://www.boost.org/" FALSE "" "Necessary to build kleopatra")


############### Windows specific ###############

if(WIN32)
  # detect oxygen icon dir at configure time based on KDEDIRS - there may be different package installation locations
  execute_process(COMMAND "${KDE4_KDECONFIG_EXECUTABLE}" --path icon OUTPUT_VARIABLE _dir ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
  file(TO_CMAKE_PATH "${_dir}" __dir)
  find_path(KDE4_ICON_DIR oxygen PATHS
    ${__dir}
  )
  message(STATUS "using oxygen application icons from ${KDE4_ICON_DIR}")
else(WIN32)
    set (KDE4_ICON_DIR  ${CMAKE_INSTALL_PREFIX}/share/icons)
endif(WIN32)


############### ONLY_KLEO ###############

# The KDEPIM_ONLY_KLEO option is true
if(KDEPIM_ONLY_KLEO)
  find_package(QGpgme)
  macro_log_feature(QGPGME_FOUND "QGpgME" "The QGpgME library" "http://www.kde.org" TRUE "" "QGpgME is required to build Kleopatra.")

  add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} ${KDEPIM_DEFINITIONS})
  include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES} ${KDEPIMLIBS_INCLUDE_DIRS} ${QT_QTDBUS_INCLUDE_DIR})
  include(kleopatra/ConfigureChecks.cmake)

  add_subdirectory(libkleo)
  if (Boost_TOPOLOGICAL_SORT_DIR)
    macro_optional_add_subdirectory(kleopatra)
  endif (Boost_TOPOLOGICAL_SORT_DIR)

else(KDEPIM_ONLY_KLEO)

# Otherwise...
############### Find the stuff we need ###############

#  find_package(Akonadi QUIET NO_MODULE)
#  macro_log_feature(Akonadi_FOUND "Akonadi" "Akonadi server libraries" "http://pim.kde.org/akonadi" TRUE "" "Akonadi is required to build KDEPIM")

  find_package(ZLIB)
  macro_log_feature(ZLIB_FOUND "ZLib" "The Zlib compression library" "http://www.zlib.net" TRUE "" "")

  find_package(Strigi)
  macro_log_feature(STRIGI_FOUND "Strigi" "Index metadata of files" "http://strigi.sourceforge.net" TRUE "" "")

  find_package(QGpgme)
  macro_log_feature(QGPGME_FOUND "QGpgME" "The QGpgME library" "http://www.kde.org" FALSE "" "QGpgME is required to build KMail, KOrganizer and Kleopatra")

  set(SDO_MIN_VERSION 0.2)
  set(SOPRANO_MIN_VERSION 2.3.70)

#  find_package(SharedDesktopOntologies ${SDO_MIN_VERSION})
#  macro_log_feature(SHAREDDESKTOPONTOLOGIES_FOUND "Shared desktop ontologies" "Desktop ontologies" "http://oscaf.sourceforge.net" TRUE "${SDO_MIN_VERSION}" "Ontologies necessary for the Nepomuk semantic desktop.")
  set(SHAREDDESKTOPONTOLOGIES_FOUND 0)

#  find_package(Soprano)
#  macro_log_feature(Soprano_FOUND "Soprano" "Semantic Desktop Storing" "http://soprano.sourceforge.net" TRUE "${SOPRANO_MIN_VERSION}" "Soprano is needed for Nepomuk")
  set(Soprano_FOUND 0)

#  find_package(Nepomuk)
#  macro_log_feature(Nepomuk_FOUND "Nepomuk" "The Nepomuk libraries" "http://www.kde.org" TRUE "" "Nepomuk extends the search and tagging functionalities in KMail and Akonadi")
  set(Nepomuk_FOUND 0)


############### Needed commands before building anything ###############

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} ${KDEPIM_DEFINITIONS})

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDEPIMLIBS_INCLUDE_DIRS} ${KDE4_INCLUDES} ${QT_QTDBUS_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/libkdepim)


############### Macros ###############

# generates a D-Bus interface description from a KConfigXT file
macro(kcfg_generate_dbus_interface _kcfg _name)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_name}.xml
    COMMAND ${XSLTPROC_EXECUTABLE} --stringparam interfaceName ${_name}
    ${KDEPIMLIBS_DATA_DIR}/akonadi-kde/kcfg2dbus.xsl
    ${_kcfg}
    > ${CMAKE_CURRENT_BINARY_DIR}/${_name}.xml
    DEPENDS ${KDEPIMLIBS_DATA_DIR}/akonadi-kde/kcfg2dbus.xsl
    ${_kcfg}
    )
endmacro(kcfg_generate_dbus_interface)


############### Code used in several places ###############
# TODO Remove from the top CMakeLists.

# used in knotes, kontact/plugins/knotes, kresources/egroupware, kresources/kolab/knotes, kresources/scalix/knotes, wizard
set(libknotesresources_SRCS
  ${CMAKE_SOURCE_DIR}/knotes/resourcemanager.cpp
  ${CMAKE_SOURCE_DIR}/knotes/resourcenotes.cpp
  ${CMAKE_SOURCE_DIR}/knotes/resourcelocal.cpp
  ${CMAKE_SOURCE_DIR}/knotes/resourcelocalconfig.cpp
)

# used in knotes, kontact/plugins/knotes
set(libknoteseditor_SRCS
  ${CMAKE_SOURCE_DIR}/knotes/knoteedit.cpp
)

# used in kdgantt1, korganizer
set(libkdgantt1_SRCS
  ${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttView.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttViewEventItem.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttViewItem.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttViewItemDrag.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttViewSubwidgets.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttViewSummaryItem.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttViewTaskItem.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttViewTaskLink.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttViewTaskLinkGroup.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttMinimizeSplitter.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttSemiSizingControl.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttSizingControl.cpp
  ${CMAKE_SOURCE_DIR}/kdgantt1/KDGanttXMLTools.cpp
)


############### Now, we add the PIM components ###############

  include (kleopatra/ConfigureChecks.cmake)

  # These targets will always be built before anything else.
#  add_subdirectory(akonadi)
  add_subdirectory(libkdepim)
  add_subdirectory(libkleo)
  add_subdirectory(libkpgp)
  add_subdirectory(libksieve)
  add_subdirectory(mimelib)
  add_subdirectory(kdgantt1)
  add_subdirectory(icons)
  add_subdirectory(messagecore)
  add_subdirectory(messagelist)

  if(QGPGME_FOUND)
    if (Boost_TOPOLOGICAL_SORT_DIR)
      macro_optional_add_subdirectory(kleopatra)
    endif(Boost_TOPOLOGICAL_SORT_DIR)
    macro_optional_add_subdirectory(wizards)
    # The following components depend on QGpgME.
    add_subdirectory(messageviewer)
    macro_optional_add_subdirectory(kmail)

    if(KDEPIM_BUILD_EXAMPLES)
      add_subdirectory(examples)
    endif(KDEPIM_BUILD_EXAMPLES)

    # If kmail is compiled, KMAIL_SUPPORTED is true (used in several places)
    if(BUILD_kmail)
      set(KMAIL_SUPPORTED TRUE)
      add_definitions(-DKMAIL_SUPPORTED)
    endif(BUILD_kmail)

    macro_optional_add_subdirectory(kmailcvt)
    macro_optional_add_subdirectory(ksendemail)
    macro_optional_add_subdirectory(korganizer)

#    include( SopranoAddOntology )
#    add_subdirectory( ontologies )
#    add_subdirectory( nepomuk_email_feeder )
  endif(QGPGME_FOUND)

  if(KDEPIM_BUILD_EVERYTHING)
    macro_optional_add_subdirectory(knode)
  endif(KDEPIM_BUILD_EVERYTHING)

  macro_optional_add_subdirectory(akregator)
#  macro_optional_add_subdirectory(akonadiconsole)
  macro_optional_add_subdirectory(blogilo)
  macro_optional_add_subdirectory(console)
  macro_optional_add_subdirectory(kalarm)
  macro_optional_add_subdirectory(kjots)
  macro_optional_add_subdirectory(knotes)
  macro_optional_add_subdirectory(strigi-analyzer)
    
  if(EXISTS ${CMAKE_SOURCE_DIR}/runtime)
    macro_optional_add_subdirectory(runtime)
  endif(EXISTS ${CMAKE_SOURCE_DIR}/runtime)

  if (BUILD_runtime)
    include_directories(${CMAKE_SOURCE_DIR}/runtime ${CMAKE_BINARY_DIR}/runtime)
  endif (BUILD_runtime)

  macro_optional_add_subdirectory(kaddressbook)
  if(Q_WS_X11)
    macro_optional_add_subdirectory(ktimetracker)
  endif(Q_WS_X11)

  macro_optional_add_subdirectory(kontact) # must be the last one.

  # These targets depend on optional applications
  add_subdirectory(kresources) # Must be after KAddressbook
  add_subdirectory(plugins) # Must be after KMail

endif (KDEPIM_ONLY_KLEO)

# doc must be a subdir of kdepim or packagers will kill us
macro_optional_add_subdirectory(doc)


# We really want to encourage users to enable/install QGpgME from kdepimlibs
if(NOT QGPGME_FOUND)
  # Users must be aware that QGpgMe is really needed
  message(STATUS "*** WARNING: QGpgME is not installed on your system ***")
  message(STATUS "*** It is required if you want to use KMail, KOrganizer or Kleopatra ***")
  message(STATUS "*** You are really encouraged to install it ***")
endif(NOT QGPGME_FOUND)
# All done, let's display what we found...
macro_display_feature_log()


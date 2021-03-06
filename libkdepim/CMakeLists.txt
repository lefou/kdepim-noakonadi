set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)
add_definitions(-DKDE_DEFAULT_DEBUG_AREA=5300)

add_subdirectory(tests)
add_subdirectory(icons)
add_subdirectory(pics)
add_subdirectory(kpartsdesignerplugin)
add_subdirectory(about)

add_definitions(${QT_QTDBUS_DEFINITIONS})


include_directories( ${Boost_INCLUDE_DIR} )

########### next target ###############

set(kdepim_LIB_SRCS
   kdescendantsproxymodel.cpp
   kcheckcombobox.cpp
   ktimeedit.cpp
   reminderclient.cpp
   kprefsdialog.cpp
   kpimprefs.cpp
   categoryselectdialog.cpp
   categoryeditdialog.cpp
   kdateedit.cpp
   kvcarddrag.cpp
   addressesdialog.cpp
   addresseeview.cpp
   maillistdrag.cpp
   kconfigpropagator.cpp
   kconfigwizard.cpp
   pluginloaderbase.cpp
   addresseelineedit.cpp
   kdatepickerpopup.cpp
   ktimezonecombobox.cpp
   ldapclient.cpp
   ldapsearchdialog.cpp
   overlaywidget.cpp
   progressmanager.cpp
   progressdialog.cpp
   statusbarprogresswidget.cpp
   ssllabel.cpp
   completionordereditor.cpp
   diffalgo.cpp
   addresseediffalgo.cpp
   calendardiffalgo.cpp
   htmldiffalgodisplay.cpp
   broadcaststatus.cpp
   kresourceprefs.cpp
   kxface.cpp
   kaccount.cpp
   kaddrbookexternal.cpp
   treewidget.cpp
   foldertreewidget.cpp
   kscoring.cpp
   kscoringeditor.cpp
   ksubscription.cpp
   kwidgetlister.cpp
   recentaddresses.cpp
   designerfields.cpp
   kcmdesignerfields.cpp
   embeddedurlpage.cpp
   kincidencechooser.cpp
   csshelper.cpp
   distributionlist.cpp
   sendsmsdialog.cpp
   categoryhierarchyreader.cpp
   kmailcompletion.cpp
   kmeditor.cpp
   autochecktreewidget.cpp
   utils.cpp
   colorlistbox.cpp
   uistatesaver.cpp
   distributionlistconverter.cpp
#   contactsfilterproxymodel.cpp
#   contactstreemodel.cpp
)

QT4_ADD_DBUS_INTERFACE(kdepim_LIB_SRCS ${CMAKE_SOURCE_DIR}/kaddressbook/org.kde.KAddressbook.Core.xml kaddressbookcore_interface )

QT4_ADD_DBUS_INTERFACE(kdepim_LIB_SRCS ${CMAKE_SOURCE_DIR}/korganizer/korgac/org.kde.korganizer.KOrgac.xml korganizer_korgac_interface )

kde4_add_ui_files(kdepim_LIB_SRCS
  categoryselectdialog_base.ui
  categoryeditdialog_base.ui
  addresspicker.ui
)

kde4_add_library(kdepim SHARED ${kdepim_LIB_SRCS})

target_link_libraries(kdepim
  ${KDE4_KDE3SUPPORT_LIBS}
  ${QT_QTDESIGNER_LIBRARY}
  ${KDEPIMLIBS_KRESOURCES_LIBS}
  ${KDEPIMLIBS_KABC_LIBS}
  ${KDEPIMLIBS_KCAL_LIBS}
  kimproxy
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  ${KDEPIMLIBS_KLDAP_LIBS}
  ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
  ${KDE4_KPARTS_LIBS}
  ${KDE4_KMIME_LIBRARY}
  ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
)

target_link_libraries(kdepim LINK_INTERFACE_LIBRARIES
  ${KDEPIMLIBS_KRESOURCES_LIBS}
  ${KDEPIMLIBS_KABC_LIBS}
  ${KDEPIMLIBS_KCAL_LIBS}
  kimproxy
  ${KDEPIMLIBS_KPIMUTILS_LIBS}
  ${KDEPIMLIBS_KLDAP_LIBS}
  ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
  ${KDE4_KPARTS_LIBS}
  ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS}
)

if(MINGW)
   target_link_libraries(kdepim oleaut32)
endif(MINGW)


set_target_properties(kdepim PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS kdepim ${INSTALL_TARGETS_DEFAULT_ARGS})


########### next target ###############

set(kdepimwidgets_PART_SRCS)
kde4_add_widget_files(kdepimwidgets_PART_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/kdepim.widgets)

kde4_add_plugin(kdepimwidgets ${kdepimwidgets_PART_SRCS})



target_link_libraries(kdepimwidgets  ${KDE4_KIO_LIBS} kdepim)

install(TARGETS kdepimwidgets  DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/designer)


########### install files ###############

install(FILES interfaces/org.kde.addressbook.service.xml interfaces/org.kde.mailtransport.service.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR})

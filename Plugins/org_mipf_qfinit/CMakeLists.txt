project(org_mipf_qfinit)

set(PLUGIN_export_directive "org_mipf_qfinit_EXPORT")

find_package(CTK REQUIRED)

include_directories(
	${CTKCore_INCLUDE_DIRS}
	${CTKPluginFramework_INCLUDE_DIRS}
)

set(PLUGIN_SRCS
  QfInitPlugin.cxx
  QfServiceInstance.cxx
)

# Files which should be processed by Qts moc
set(PLUGIN_MOC_SRCS
  QfInitPlugin_p.h
  QfServiceInstance.h
  QfService.h
)

# Qt Designer files which should be processed by Qts uic
set(PLUGIN_UI_FORMS
)

# QRC Files which should be compiled into the plugin
set(PLUGIN_resources
)

#Compute the plugin dependencies
ctkFunctionGetTargetLibraries(PLUGIN_target_libraries)

ctkMacroBuildPlugin(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${PLUGIN_export_directive}
  SRCS ${PLUGIN_SRCS}
  MOC_SRCS ${PLUGIN_MOC_SRCS}
  UI_FORMS ${PLUGIN_UI_FORMS}
  RESOURCES ${PLUGIN_resources}
  TARGET_LIBRARIES ${PLUGIN_target_libraries} ${QTFRAMEWORK_LIBRARIES} ${QFMAIN_LIBRARIES}
)
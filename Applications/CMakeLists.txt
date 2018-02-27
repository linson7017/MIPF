cmake_minimum_required(VERSION 2.8.8)

macro(source_group_by_dir source_files)
    if(MSVC)
        set(sgbd_cur_dir ${CMAKE_CURRENT_SOURCE_DIR})
        foreach(sgbd_file ${${source_files}})
            string(REGEX REPLACE ${sgbd_cur_dir}/\(.*\) \\1 sgbd_fpath ${sgbd_file})
            string(REGEX REPLACE "\(.*\)/.*" \\1 sgbd_group_name ${sgbd_fpath})
            string(COMPARE EQUAL ${sgbd_fpath} ${sgbd_group_name} sgbd_nogroup)
            string(REPLACE "/" "\\" sgbd_group_name ${sgbd_group_name})
            if(sgbd_nogroup)
                set(sgbd_group_name "\\")
				message("No group")
            endif(sgbd_nogroup)
			message(${sgbd_group_name})
            source_group(${sgbd_group_name} FILES ${sgbd_file})
        endforeach(sgbd_file)
    endif(MSVC)
endmacro(source_group_by_dir)

#将qfconfig目录下的配置文件加入工程中
macro(MipfGetQfConfig projectname QfConfigFiles)
	set(qfconfig_dir ${CMAKE_SOURCE_DIR}/bin/qfconfig/${projectname})
	##configfiles
	file(GLOB qfconfig_config_files "${qfconfig_dir}/*.cfg")
	source_group("config" FILES ${qfconfig_config_files})
	##mitk
	file(GLOB qfconfig_mitk_interactions_files "${qfconfig_dir}/mitk/Interactions/*.xml")
	source_group("mitk_interactions" FILES ${qfconfig_mitk_interactions_files})
	##qtframework
	file(GLOB qfconfig_qtframework_layout_files "${qfconfig_dir}/res/layout/*.xml")
	source_group("qtframework_layout" FILES ${qfconfig_qtframework_layout_files})
	file(GLOB qfconfig_qtframework_ui_files "${qfconfig_dir}/res/ui/*.xml")
	source_group("qtframework_ui" FILES ${qfconfig_qtframework_ui_files})
	file(GLOB qfconfig_qtframework_styles_files "${qfconfig_dir}/res/styles/*.xml")
	source_group("qtframework_styles" FILES ${qfconfig_qtframework_styles_files})
	file(GLOB qfconfig_qtframework_uicommands_files "${qfconfig_dir}/res/uicommands/*.xml")
	source_group("qtframework_uicommands" FILES ${qfconfig_qtframework_uicommands_files})
	
	set(QfConfigFiles 
	${qfconfig_config_files} 
	${qfconfig_mitk_interactions_files} 
	${qfconfig_qtframework_layout_files}
	${qfconfig_qtframework_ui_files}
	${qfconfig_qtframework_styles_files}
	${qfconfig_qtframework_uicommands_files})
	source_group("qfconfig" FILES ${QfConfigFiles} )
endmacro(MipfGetQfConfig)

#拷贝qfconfig文件到运行目录下
macro(InstallQfConfig projectname)
# add_custom_command(TARGET ${projectname} POST_BUILD
                  # COMMAND ${CMAKE_COMMAND} -E copy_directory
                  # ${CMAKE_SOURCE_DIR}/bin/qfconfig/${projectname} ${CMAKE_BINARY_DIR}/bin/qfconfig/${projectname})
add_custom_target(Copy_${projectname}_Resources ALL
				  COMMAND echo "copying ${CMAKE_SOURCE_DIR}/bin/qfconfig/${projectname} to ${CMAKE_BINARY_DIR}/bin/qfconfig/${projectname}"
				  COMMAND ${CMAKE_COMMAND} -E copy_directory
				  ${CMAKE_SOURCE_DIR}/bin/qfconfig/${projectname} ${CMAKE_BINARY_DIR}/bin/qfconfig/${projectname})
endmacro(InstallQfConfig) 

add_subdirectory(WorkBench)
add_subdirectory(PluginGenerator)
add_subdirectory(RobotControlApp)
add_subdirectory(ModuleGenerator)
add_subdirectory(MapperTest)
add_subdirectory(TestApp)


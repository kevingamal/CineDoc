

set(command "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe;-DCMAKE_INSTALL_PREFIX=C:/Users/Sergio/informatica/CineDoc/stage;-DCMAKE_BUILD_TYPE=;-DwxBUILD_SHARED=OFF;-GVisual Studio 15 2017;-DCMAKE_GENERATOR_INSTANCE:INTERNAL=C:/Program Files (x86)/Microsoft Visual Studio/2019/Community;C:/Users/Sergio/informatica/CineDoc/subprojects/Source/wxWidgets_external")
execute_process(
  COMMAND ${command}
  RESULT_VARIABLE result
  OUTPUT_FILE "C:/Users/Sergio/informatica/CineDoc/subprojects/Stamp/wxWidgets_external/wxWidgets_external-configure-out.log"
  ERROR_FILE "C:/Users/Sergio/informatica/CineDoc/subprojects/Stamp/wxWidgets_external/wxWidgets_external-configure-err.log"
  )
if(result)
  set(msg "Command failed: ${result}\n")
  foreach(arg IN LISTS command)
    set(msg "${msg} '${arg}'")
  endforeach()
  set(msg "${msg}\nSee also\n  C:/Users/Sergio/informatica/CineDoc/subprojects/Stamp/wxWidgets_external/wxWidgets_external-configure-*.log")
  message(FATAL_ERROR "${msg}")
else()
  set(msg "wxWidgets_external configure command succeeded.  See also C:/Users/Sergio/informatica/CineDoc/subprojects/Stamp/wxWidgets_external/wxWidgets_external-configure-*.log")
  message(STATUS "${msg}")
endif()

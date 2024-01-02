

set(command "C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe;--build;.;--config;Release")
execute_process(
  COMMAND ${command}
  RESULT_VARIABLE result
  OUTPUT_FILE "C:/Users/Sergio/informatica/CineDoc/subprojects/Stamp/wxWidgets_external/wxWidgets_external-build-out.log"
  ERROR_FILE "C:/Users/Sergio/informatica/CineDoc/subprojects/Stamp/wxWidgets_external/wxWidgets_external-build-err.log"
  )
if(result)
  set(msg "Command failed: ${result}\n")
  foreach(arg IN LISTS command)
    set(msg "${msg} '${arg}'")
  endforeach()
  set(msg "${msg}\nSee also\n  C:/Users/Sergio/informatica/CineDoc/subprojects/Stamp/wxWidgets_external/wxWidgets_external-build-*.log")
  message(FATAL_ERROR "${msg}")
else()
  set(msg "wxWidgets_external build command succeeded.  See also C:/Users/Sergio/informatica/CineDoc/subprojects/Stamp/wxWidgets_external/wxWidgets_external-build-*.log")
  message(STATUS "${msg}")
endif()

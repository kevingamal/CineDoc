# BUILD COMMANDS:
# cmake -S. -Bbuild
# cmake --build build

cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

project(wx_cmake_template_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# hack for buggy CMake's FindwxWidgets script
if(DEFINED ENV_WX_CONFIG)
    set(ENV{WX_CONFIG} ${ENV_WX_CONFIG})
endif()

set(wxWidgets_USE_STATIC 1)

# Buscar wxWidgets y Boost
find_package(wxWidgets REQUIRED)

# if(WIN32)
#     set(BOOST_ROOT "C:/Boost")
#     set(Boost_INCLUDE_DIR "C:/Boost/include")
#     set(Boost_LIBRARY_DIR "C:/Boost/lib")
# endif()

# cmake_policy(SET CMP0074 NEW)
# find_package(Boost 1.83 REQUIRED COMPONENTS serialization)

include(${wxWidgets_USE_FILE})
# include_directories(${Boost_INCLUDE_DIRS})

# Colocar cualquier otro nombre_de_archivo.cpp para conmutar el archivo a compilar
set(SRCS main.cpp)

if(APPLE)
    # create bundle on apple compiles
    add_executable(main MACOSX_BUNDLE ${SRCS})

    # Set a custom plist file for the app bundle - needed for Mac OS Retina display
    set_target_properties(main PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
else()
    # the WIN32 is needed for Windows in order for it to look for WinMain
    # instead of the main function. This is ignored on other systems,
    # so it works on all platforms
    add_executable(main WIN32 ${SRCS} main.exe.manifest)
endif()

# Vincular tanto las bibliotecas wxWidgets como las de Boost
# target_link_libraries(main PRIVATE ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES})
target_link_libraries(main PRIVATE ${wxWidgets_LIBRARIES})
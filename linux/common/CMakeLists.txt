# CMakeList.txt for common
#   Build a library (`common`) which exposes the header files as "common/*.h"
#   Use header as: #include "common/button.h"

include_directories(common/include)
file(GLOB MY_SOURCES "src/*.c")

add_library(common STATIC ${MY_SOURCES})

# ALSA support
find_package(ALSA REQUIRED)
target_link_libraries(common LINK_PRIVATE asound)

# Make use of the libraries
target_include_directories(common PUBLIC include)

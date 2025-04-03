# Build the app, using the HAL

include_directories(include)
file(GLOB MY_SOURCES "src/*.c")
add_executable(beatbox ${MY_SOURCES})

# Make use of the libraries
target_link_libraries(beatbox LINK_PRIVATE hal)
target_link_libraries(beatbox LINK_PRIVATE common)
target_link_libraries(beatbox LINK_PRIVATE lcd)
target_link_libraries(beatbox LINK_PRIVATE lgpio)

# Copy executable to final location (change `beatbox` to project name as needed)
add_custom_command(TARGET beatbox POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy 
     "$<TARGET_FILE:beatbox>"
     "~/cmpt433/public/myApps/beatbox"
  COMMENT "Copying ARM executable to public NFS directory")

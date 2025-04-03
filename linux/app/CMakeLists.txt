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

# Copy the WAV folder to NFS
add_custom_command(TARGET beatbox POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy_directory
     "${CMAKE_SOURCE_DIR}/beatbox-wav-files"
     "~/cmpt433/public/myApps/beatbox-wav-files" 
  COMMENT "Copying WAVE files to public NFS directory")

# Copy the nodejs server folder to NFS
add_custom_command(TARGET beatbox POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy_directory
     "${CMAKE_SOURCE_DIR}/beatbox-server-copy"
     "~/cmpt433/public/myApps/beatbox-server-copy" 
  COMMENT "Copying Node.js server to public NFS directory")

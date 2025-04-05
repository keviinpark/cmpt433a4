# Build the app, using the HAL

include_directories(include)
file(GLOB MY_SOURCES "src/*.c")
add_executable(finddot ${MY_SOURCES})

# Make use of the libraries
target_link_libraries(finddot LINK_PRIVATE hal m)
target_link_libraries(finddot LINK_PRIVATE common)
target_link_libraries(finddot LINK_PRIVATE lcd)
target_link_libraries(finddot LINK_PRIVATE lgpio)

# Copy executable to final location (change `finddot` to project name as needed)
add_custom_command(TARGET finddot POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy 
     "$<TARGET_FILE:finddot>"
     "~/cmpt433/public/myApps/finddot"
  COMMENT "Copying ARM executable to public NFS directory")

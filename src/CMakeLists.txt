cmake_minimum_required( VERSION 3.6 )

# Create Project
project( imageio_sample CXX )

if(WIN32)
  message("OS is Windows")
  # Set StartUp Project
  set_property( DIRECTORY PROPERTY VS_STARTUP_PROJECT "imageio_sample" )

elseif(UNIX)
  message("OS is UNIX")

elseif(APPLE)
  message("OS is APPLE")

elseif(WIN32)
  message("OS is Unknown")

endif(WIN32)

# Add Executable
add_executable( imageio_sample main.cpp ImageIO.h ImageIO.cpp )

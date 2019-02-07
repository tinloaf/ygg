cmake_minimum_required(VERSION 3.0.2)
project(draup_fetcher C CXX)
include(ExternalProject)

set(CMAKE_CXX_STANDARD 17)

ExternalProject_Add(draup
  GIT_REPOSITORY    https://github.com/tinloaf/draup.git
	GIT_TAG           43c45b5523a49078081ebe68011604192e4543af
	PREFIX           "${CMAKE_CURRENT_BINARY_DIR}"
  # We just want to download
	BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  )

# Specify include dir
ExternalProject_Get_Property(draup source_dir)
set(DRAUP_INCLUDE_DIRS ${source_dir})

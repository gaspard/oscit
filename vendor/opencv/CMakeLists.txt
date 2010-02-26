# ----------------------------------------------------------------------------
#  CMake file for matrix allocation using opencv cv::Mat.
#  See root CMakeLists.txt
#
# ----------------------------------------------------------------------------
PROJECT(opencv_mat)

# List of C++ files:

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB lib_srcs *.cpp)
file(GLOB lib_hdrs *.h)
set(lib_ext_hdrs)

set(the_target "opencv_mat")

add_library(${the_target} STATIC ${lib_srcs} ${lib_hdrs} ${lib_ext_hdrs})

if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3")
endif()

if(UNIX)
  if(CMAKE_COMPILER_IS_GNUCXX OR CV_ICC)
     set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  endif()
endif()

set_target_properties(${the_target}
    PROPERTIES
        OUTPUT_NAME "${the_target}"
      	ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/vendor_lib
    )

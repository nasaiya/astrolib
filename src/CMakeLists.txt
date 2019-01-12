cmake_minimum_required(VERSION 3.0.0)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_BUILD_DIR ${PROJECT_SOURCE_DIR}/build)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/headers)
include_directories(/usr/include/swiss-ephemeris-2.07.01)


#Add any source files that will create a single library
set(astrolib_LIB
    astrolib.cpp
)

set (astrolib_INSTALL_HEADERS
    headers/astrolib.h
)



MESSAGE( STATUS "WARNING: Enabling Error output!" )
MESSAGE( STATUS "WARNING: Enabling warnings!" )
add_definitions(
  -D__ERRORS="true"
  -D__WARNINGS="true"
)
  
if (ENABLE_DEBUG_CODE)
  MESSAGE( STATUS "WARNING: Enabling debug code!" )
  MESSAGE( STATUS "WARNING: Enabling DEEP debug code!" )
  add_definitions(
    -D__DEBUG="true" 
    -D__DDEBUG="true"
  )
endif()

# FORCE MOC to run on the headers/*.h files... without this it's just undefined references to everything in those headers.
qt5_wrap_cpp(astrolib_MOCEXHDRS ${cryptapi_INSTALL_HEADERS})

if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
	execute_process(
		COMMAND git rev-parse HEAD
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE GIT_BRANCH
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
else()
	set(GIT_BRANCH "Unknown")
endif()
add_definitions(-DGIT_BRANCH=${GIT_BRANCH})
#set build dir for your library
set(LIBRARY_OUTPUT_PATH ${CMAKE_BUILD_DIR}/lib)
message( STATUS "LIBRARY_OUTPUT_PATH: " ${LIBRARY_OUTPUT_PATH} )

set(LIBRARY_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/lib)
message( STATUS "LIBRARY_INSTALL_PATH: " ${LIBRARY_INSTALL_PATH} )

set(HEADERS_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/include/astrolib)
message(STATUS "HEADERS_INSTALL_PATH : " ${HEADERS_INSTALL_PATH})

file(MAKE_DIRECTORY ${LIBRARY_OUTPUT_PATH})

# Tell CMake to create the astrolib shared lib and set it's properties
add_library(astrolib SHARED ${astrolib_LIB} ${astrolib_MOCEXHDRS})
set_target_properties(astrolib
    PROPERTIES
    VERSION 0.1
    SOVERSION 0
    )

#If your app, links to an external lib -ie Boost
#target_link_libraries( astrolib ${Boost_LIBRARIES} )
target_link_libraries( astrolib Qt5::Core swe )

# make sure we use the correct standard
set_property(TARGET astrolib PROPERTY CXX_STANDARD 17)

# Install
configure_file(astrolib-config.in.cmake "${CMAKE_CURRENT_BINARY_DIR}/astrolib-config.cmake" @ONLY)

install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/astrolib-config.cmake"
    DESTINATION lib64/astrolib
)

install(TARGETS astrolib
    EXPORT astrolib-targets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib64
    ARCHIVE DESTINATION lib64
)

install(EXPORT astrolib-targets
    FILE astrolib-targets.cmake
    DESTINATION lib64/astrolib
)

install(FILES ${cryptapi_INSTALL_HEADERS}
  DESTINATION include/astrolib
)

add_subdirectory(headers)

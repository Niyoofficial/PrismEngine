# Install script for directory: F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/_deps/fmt-src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/spdlog")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/bin/Debug/fmt.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/bin/Release/fmt.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/bin/MinSizeRel/fmt.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/bin/RelWithDebInfo/fmt.lib")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/fmt" TYPE FILE FILES
    "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/_deps/fmt-src/include/fmt/args.h"
    "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/_deps/fmt-src/include/fmt/chrono.h"
    "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/_deps/fmt-src/include/fmt/color.h"
    "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/_deps/fmt-src/include/fmt/compile.h"
    "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/_deps/fmt-src/include/fmt/core.h"
    "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/_deps/fmt-src/include/fmt/format.h"
    "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/_deps/fmt-src/include/fmt/format-inl.h"
    "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/_deps/fmt-src/include/fmt/os.h"
    "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/_deps/fmt-src/include/fmt/ostream.h"
    "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/_deps/fmt-src/include/fmt/printf.h"
    "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/_deps/fmt-src/include/fmt/ranges.h"
    "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/_deps/fmt-src/include/fmt/std.h"
    "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/_deps/fmt-src/include/fmt/xchar.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt" TYPE FILE FILES
    "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/_deps/fmt-build/fmt-config.cmake"
    "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/_deps/fmt-build/fmt-config-version.cmake"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt/fmt-targets.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt/fmt-targets.cmake"
         "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/_deps/fmt-build/CMakeFiles/Export/lib/cmake/fmt/fmt-targets.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt/fmt-targets-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt/fmt-targets.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt" TYPE FILE FILES "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/_deps/fmt-build/CMakeFiles/Export/lib/cmake/fmt/fmt-targets.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt" TYPE FILE FILES "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/_deps/fmt-build/CMakeFiles/Export/lib/cmake/fmt/fmt-targets-debug.cmake")
  endif()
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt" TYPE FILE FILES "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/_deps/fmt-build/CMakeFiles/Export/lib/cmake/fmt/fmt-targets-minsizerel.cmake")
  endif()
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt" TYPE FILE FILES "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/_deps/fmt-build/CMakeFiles/Export/lib/cmake/fmt/fmt-targets-relwithdebinfo.cmake")
  endif()
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/fmt" TYPE FILE FILES "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/_deps/fmt-build/CMakeFiles/Export/lib/cmake/fmt/fmt-targets-release.cmake")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/_deps/fmt-build/fmt.pc")
endif()


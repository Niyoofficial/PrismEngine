# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.23.0-rc1)

# We name the project and the target for the ExternalProject_Add() call
# to something that will highlight to the user what we are working on if
# something goes wrong and an error message is produced.

project(fmt-populate NONE)



include(ExternalProject)
ExternalProject_Add(fmt-populate
                     "UPDATE_DISCONNECTED" "False" "DOWNLOAD_EXTRACT_TIMESTAMP" "FALSE" "URL" "https://github.com/fmtlib/fmt/archive/refs/tags/10.2.1.tar.gz" "URL_HASH" "SHA256=1250e4cc58bf06ee631567523f48848dc4596133e163f02615c97f78bab6c811"
                    SOURCE_DIR          "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/_deps/fmt-src"
                    BINARY_DIR          "F:/VisualStudio/PrismEngine/Vendor/spdlog/New folder/spdlog/VS2022/_deps/fmt-build"
                    CONFIGURE_COMMAND   ""
                    BUILD_COMMAND       ""
                    INSTALL_COMMAND     ""
                    TEST_COMMAND        ""
                    USES_TERMINAL_DOWNLOAD  YES
                    USES_TERMINAL_UPDATE    YES
                    USES_TERMINAL_PATCH     YES
)



# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/yolkee/прога торрент/Yolkee/project-part-2/peers-request/cmake-build/_deps/cpr-src"
  "/Users/yolkee/прога торрент/Yolkee/project-part-2/peers-request/cmake-build/_deps/cpr-build"
  "/Users/yolkee/прога торрент/Yolkee/project-part-2/peers-request/cmake-build/_deps/cpr-subbuild/cpr-populate-prefix"
  "/Users/yolkee/прога торрент/Yolkee/project-part-2/peers-request/cmake-build/_deps/cpr-subbuild/cpr-populate-prefix/tmp"
  "/Users/yolkee/прога торрент/Yolkee/project-part-2/peers-request/cmake-build/_deps/cpr-subbuild/cpr-populate-prefix/src/cpr-populate-stamp"
  "/Users/yolkee/прога торрент/Yolkee/project-part-2/peers-request/cmake-build/_deps/cpr-subbuild/cpr-populate-prefix/src"
  "/Users/yolkee/прога торрент/Yolkee/project-part-2/peers-request/cmake-build/_deps/cpr-subbuild/cpr-populate-prefix/src/cpr-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/yolkee/прога торрент/Yolkee/project-part-2/peers-request/cmake-build/_deps/cpr-subbuild/cpr-populate-prefix/src/cpr-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/yolkee/прога торрент/Yolkee/project-part-2/peers-request/cmake-build/_deps/cpr-subbuild/cpr-populate-prefix/src/cpr-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()

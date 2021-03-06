# OpenDLV - Software for driverless vehicles realized with OpenDaVINCI
# Copyright (C) 2016 Chalmers REVERE
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

cmake_minimum_required(VERSION 2.8)

# To create a proper build environment, you are supposed to call CMake as follows:
#
# PATH=/opt/od4/bin:$PATH cmake -D CMAKE_INSTALL_PREFIX=<Where shall OpenDLV be installed?> 
#                               -D OPENDAVINCI_DIR=<Where is OpenDaVINCI installed?> 
#                               -D CXXTEST_INCLUDE_DIR=<Where is CxxTest installed?> ..
#
# The adjusted PATH variable is required to find odDataStructureGenerator-latest.jar.
# CMAKE_INSTALL_PREFIX specifies where to install OpenDLV.
# OPENDAVINCI_DIR specifies where OpenDaVINCI is installed (if different from /usr/include and /usr/lib).
# CXXTEST_INCLUDE_DIR specifies where the UnitTest environment CxxTest can be found.

project(opendlv.core)

###########################################################################
# If dpkg and rpmbuild are available, .deb and .rpm packages will be
# created. Otherwise, the software is simply installed.
IF(     NOT("${DPKG_EXECUTABLE}" STREQUAL "DPKG_EXECUTABLE-NOTFOUND")
    AND NOT("${RPMBUILD_EXECUTABLE}" STREQUAL "RPMBUILD_EXECUTABLE-NOTFOUND") )
    MESSAGE(STATUS "Enabling building .deb and .rpm packages.")
    SET(MAKE_PACKAGE package)
ELSE()
    # If the platform does not provide dpkg and rpmbuild, make package is
    # replaced with make install.
    SET(MAKE_PACKAGE install)
ENDIF()

###########################################################################
# Set the search path for .cmake files.
SET (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake.Modules" ${CMAKE_MODULE_PATH})
SET (CMAKE_MODULE_PATH "${OPENDAVINCI_DIR}/share/cmake-${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}/Modules" ${CMAKE_MODULE_PATH})

###########################################################################
# Enable the configuration of external projects, as used below.
include(ExternalProject)

###########################################################################
# Java is needed to run the odDataStructureGenerator tool
# for generating the data structures.
include(FindJava REQUIRED)

###########################################################################
# Find odDataStructureGenerator.
include(FindodDataStructureGenerator REQUIRED)

###########################################################################
# Find odCANDataStructureGenerator.
include(FindodCANDataStructureGenerator REQUIRED)

###########################################################################
# Find OpenDaVINCI.
find_package(OpenDaVINCI REQUIRED)

###########################################################################
# Find CxxTest.
if("${CXXTEST_INCLUDE_DIR}" STREQUAL "")
  message(STATUS "No CXXTEST_INCLUDE_DIR was supplied, using the default (./thirdparty/cxxtest) directory.")
  set(CXXTEST_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/thirdparty/cxxtest) 
endif()
include(CheckCxxTestEnvironment REQUIRED)

###########################################################################
# Global project variables.
set(INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
message(STATUS "Installing OpenDLV.core to ${INSTALL_DIR}.")
set(PROJECT_ROOT ${CMAKE_SOURCE_DIR})

###########################################################################
# This "external project" simply deletes the existing libodvdopendlvdatamodel.
ExternalProject_Add (libodvdopendlvdatamodel-cleanup
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/libodvdopendlvdatamodel
    PATCH_COMMAND ""
    SOURCE_DIR ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    TEST_COMMAND ""
    INSTALL_COMMAND "")

# This "external project" builds libodvdopendlvdatamodel.
ExternalProject_Add (libodvdopendlvdatamodel
    DEPENDS libodvdopendlvdatamodel-cleanup
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND cd ${CMAKE_CURRENT_BINARY_DIR} && ${Java_JAVA_EXECUTABLE} -jar ${ODDATASTRUCTUREGENERATOR_EXECUTABLE} --withCMake ${CMAKE_CURRENT_SOURCE_DIR}/resources/odvd/ODVDOpenDLVDataModel.odvd
    PATCH_COMMAND ""
    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/libodvdopendlvdatamodel"
    CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}"
    CMAKE_ARGS "-DOPENDAVINCI_DIR=${OPENDAVINCI_DIR}"
    CMAKE_ARGS "-DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE}"
    CMAKE_ARGS "-DCXXTEST_INCLUDE_DIR=${CXXTEST_INCLUDE_DIR}"
    TEST_BEFORE_INSTALL 1
    TEST_COMMAND ${CMAKE_CTEST_COMMAND} ${CTEST_PARAMETERS}
    INSTALL_COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/libodvdopendlvdatamodel-prefix/src/libodvdopendlvdatamodel-build --target install COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/libodvdopendlvdatamodel-prefix/src/libodvdopendlvdatamodel-build --target ${MAKE_PACKAGE})

###########################################################################
# This "external project" simply deletes the existing libodvdapplanix.
ExternalProject_Add (libodvdapplanix-cleanup
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/libodvdapplanix
    PATCH_COMMAND ""
    SOURCE_DIR ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    TEST_COMMAND ""
    INSTALL_COMMAND "")

# This "external project" builds libodvdapplanix.
ExternalProject_Add (libodvdapplanix
    DEPENDS libodvdapplanix-cleanup
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND cd ${CMAKE_CURRENT_BINARY_DIR} && ${Java_JAVA_EXECUTABLE} -jar ${ODDATASTRUCTUREGENERATOR_EXECUTABLE} --withCMake ${CMAKE_CURRENT_SOURCE_DIR}/resources/odvd/ODVDApplanix.odvd
    PATCH_COMMAND ""
    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/libodvdapplanix"
    CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}"
    CMAKE_ARGS "-DOPENDAVINCI_DIR=${OPENDAVINCI_DIR}"
    CMAKE_ARGS "-DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE}"
    CMAKE_ARGS "-DCXXTEST_INCLUDE_DIR=${CXXTEST_INCLUDE_DIR}"
    TEST_BEFORE_INSTALL 1
    TEST_COMMAND ${CMAKE_CTEST_COMMAND} ${CTEST_PARAMETERS}
    INSTALL_COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/libodvdapplanix-prefix/src/libodvdapplanix-build --target install COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/libodvdapplanix-prefix/src/libodvdapplanix-build --target ${MAKE_PACKAGE})

###########################################################################
# This "external project" simply deletes the existing libodvdvehicle.
ExternalProject_Add (libodvdvehicle-cleanup
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/libodvdvehicle
    PATCH_COMMAND ""
    SOURCE_DIR ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    TEST_COMMAND ""
    INSTALL_COMMAND "")

# This "external project" builds libodvdvehicle.
ExternalProject_Add (libodvdvehicle
    DEPENDS libodvdvehicle-cleanup
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND cd ${CMAKE_CURRENT_BINARY_DIR} && ${Java_JAVA_EXECUTABLE} -jar ${ODDATASTRUCTUREGENERATOR_EXECUTABLE} --withCMake ${CMAKE_CURRENT_SOURCE_DIR}/resources/odvd/ODVDVehicle.odvd
    PATCH_COMMAND ""
    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/libodvdvehicle"
    CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}"
    CMAKE_ARGS "-DOPENDAVINCI_DIR=${OPENDAVINCI_DIR}"
    CMAKE_ARGS "-DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE}"
    CMAKE_ARGS "-DCXXTEST_INCLUDE_DIR=${CXXTEST_INCLUDE_DIR}"
    TEST_BEFORE_INSTALL 1
    TEST_COMMAND ${CMAKE_CTEST_COMMAND} ${CTEST_PARAMETERS}
    INSTALL_COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/libodvdvehicle-prefix/src/libodvdvehicle-build --target install COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/libodvdvehicle-prefix/src/libodvdvehicle-build --target ${MAKE_PACKAGE})

###########################################################################
# This "external project" simply deletes the existing libodvdfh16truck.
ExternalProject_Add (libodvdfh16truck-cleanup
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/libodvdfh16truck
    PATCH_COMMAND ""
    SOURCE_DIR ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    TEST_COMMAND ""
    INSTALL_COMMAND "")

# This "external project" builds libodvdfh16truck.
ExternalProject_Add (libodvdfh16truck
    DEPENDS libodvdfh16truck-cleanup
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND cd ${CMAKE_CURRENT_BINARY_DIR} && ${Java_JAVA_EXECUTABLE} -jar ${ODDATASTRUCTUREGENERATOR_EXECUTABLE} --withCMake ${CMAKE_CURRENT_SOURCE_DIR}/resources/odvd/ODVDFH16Truck.odvd
    PATCH_COMMAND ""
    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/libodvdfh16truck"
    CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}"
    CMAKE_ARGS "-DOPENDAVINCI_DIR=${OPENDAVINCI_DIR}"
    CMAKE_ARGS "-DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE}"
    CMAKE_ARGS "-DCXXTEST_INCLUDE_DIR=${CXXTEST_INCLUDE_DIR}"
    TEST_BEFORE_INSTALL 1
    TEST_COMMAND ${CMAKE_CTEST_COMMAND} ${CTEST_PARAMETERS}
    INSTALL_COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/libodvdfh16truck-prefix/src/libodvdfh16truck-build --target install COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/libodvdfh16truck-prefix/src/libodvdfh16truck-build --target ${MAKE_PACKAGE})

###########################################################################
# This "external project" simply deletes the existing libfh16mapping.
ExternalProject_Add(libfh16mapping-cleanup
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/libfh16mapping
    PATCH_COMMAND ""
    SOURCE_DIR ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    TEST_COMMAND ""
    INSTALL_COMMAND "")

# This "external project" builds libfh16mapping.
ExternalProject_Add(libfh16mapping
    DEPENDS libfh16mapping-cleanup libodvdfh16truck
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND cd ${CMAKE_CURRENT_BINARY_DIR} && ${Java_JAVA_EXECUTABLE} -jar ${ODCANDATASTRUCTUREGENERATOR_EXECUTABLE} --withCMake ${CMAKE_SOURCE_DIR}/resources/can/fh16mapping.can
    PATCH_COMMAND ""
    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/libfh16mapping"
    CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}"
    CMAKE_ARGS "-DOPENDAVINCI_DIR=${OPENDAVINCI_DIR}"
    CMAKE_ARGS "-DCXXTEST_INCLUDE_DIR=${CXXTEST_INCLUDE_DIR}"
    TEST_BEFORE_INSTALL 1
    INSTALL_COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/libfh16mapping-prefix/src/libfh16mapping-build --target install COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/libfh16mapping-prefix/src/libfh16mapping-build --target ${MAKE_PACKAGE})

###########################################################################
# This "external project" simply deletes the existing libodvdtrimble.
ExternalProject_Add (libodvdtrimble-cleanup
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/libodvdtrimble
    PATCH_COMMAND ""
    SOURCE_DIR ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    TEST_COMMAND ""
    INSTALL_COMMAND "")

# This "external project" builds libodvdtrimble.
ExternalProject_Add (libodvdtrimble
    DEPENDS libodvdtrimble-cleanup
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND cd ${CMAKE_CURRENT_BINARY_DIR} && ${Java_JAVA_EXECUTABLE} -jar ${ODDATASTRUCTUREGENERATOR_EXECUTABLE} --withCMake ${CMAKE_CURRENT_SOURCE_DIR}/resources/odvd/ODVDTrimble.odvd
    PATCH_COMMAND ""
    SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/libodvdtrimble"
    CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}"
    CMAKE_ARGS "-DOPENDAVINCI_DIR=${OPENDAVINCI_DIR}"
    CMAKE_ARGS "-DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_FILE}"
    CMAKE_ARGS "-DCXXTEST_INCLUDE_DIR=${CXXTEST_INCLUDE_DIR}"
    TEST_BEFORE_INSTALL 1
    TEST_COMMAND ${CMAKE_CTEST_COMMAND} ${CTEST_PARAMETERS}
    INSTALL_COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/libodvdtrimble-prefix/src/libodvdtrimble-build --target install COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/libodvdtrimble-prefix/src/libodvdtrimble-build --target ${MAKE_PACKAGE})

###########################################################################
# This "external project" builds and installs the system software stack.
ExternalProject_Add(system
    DEPENDS libodvdapplanix libodvdvehicle libodvdfh16truck libfh16mapping libodvdtrimble libodvdopendlvdatamodel
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    SOURCE_DIR "${CMAKE_SOURCE_DIR}/code/core/system"
    CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}"
    CMAKE_ARGS "-DOPENDAVINCI_DIR=${OPENDAVINCI_DIR}"
    CMAKE_ARGS "-DCXXTEST_INCLUDE_DIR=${CXXTEST_INCLUDE_DIR}"
    TEST_BEFORE_INSTALL 1
    INSTALL_COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/system-prefix/src/system-build --target install COMMAND ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/system-prefix/src/system-build --target ${MAKE_PACKAGE})


# Copyright (C) 2013 The Regents of the University of California (Regents).
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#     * Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#
#     * Neither the name of The Regents or University of California nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# Please contact the author of this library if you have any questions.
# Author: Chris Sweeney (cmsweeney@cs.ucsb.edu)
#
# Macro for tests
#   Ex: Your test is written in myclass_test.cc  and uses a class in myclass.cc
#       which has been compiled as target: myclass then calling
#       GTEST(myclass_test myclass) will create a test executable myclass_test
#       by automatically compiling myclass_test.cc and linking the mytest target
MACRO (GTEST NAME)
  ADD_EXECUTABLE(${NAME} ${CMAKE_SOURCE_DIR}/test/test_main.cc ${NAME}.cc)
  TARGET_LINK_LIBRARIES(${NAME} gtest ${ARGN} ${THEIA_LIBRARY_DEPENDENCIES})
  ADD_TEST(NAME ${NAME}
           COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME})
ENDMACRO (GTEST)

# Add c++ macro to get test data directory
ADD_DEFINITIONS(-DTHEIA_TEST_DATA_DIR="${CMAKE_SOURCE_DIR}/test/data")
SET(THEIA_SRC_FILES CACHE INTERNAL "include files" FORCE)
SET(THEIA_SRC_DEPS CACHE INTERNAL "include deps" FORCE)
# Macro for CC Libraries.
MACRO (CC_LIBRARY NAME SRCS)
  STRING(REPLACE "${CMAKE_SOURCE_DIR}/" "" TARGETDIR ${CMAKE_CURRENT_LIST_DIR})
  ADD_LIBRARY(${TARGETDIR}/${NAME} SHARED ${SRCS})
  SET_PROPERTY(TARGET ${TARGETDIR}/${NAME} PROPERTY OUTPUT_NAME ${NAME})
  SET_PROPERTY(TARGET ${TARGETDIR}/${NAME} PROPERTY LINKER_LANGUAGE CXX)
  TARGET_LINK_LIBRARIES(${TARGETDIR}/${NAME} ${ARGN} ${THEIA_LIBRARY_DEPENDENCIES})
  get_filename_component(ABS_PATH ${SRCS} ABSOLUTE)
  list(FIND THEIA_SRC_FILES ${ABS_PATH} _contains_already)
  SET(THEIA_SRC_FILES ${THEIA_SRC_FILES} ${ABS_PATH} CACHE INTERNAL "include files")
  SET(THEIA_SRC_DEPS ${THEIA_SRC_DEPS} ${TARGETDIR}/${NAME} ${ARGN} CACHE INTERNAL "include files")
ENDMACRO (CC_LIBRARY)

# Macro for Header Libraries.
MACRO (HEADER_LIBRARY NAME SRCS)
  STRING(REPLACE "${CMAKE_SOURCE_DIR}/" "" TARGETDIR ${CMAKE_CURRENT_LIST_DIR})
  ADD_LIBRARY(${TARGETDIR}/${NAME} SHARED ${CMAKE_SOURCE_DIR}/util/dummy.cc)
  SET_PROPERTY(TARGET ${TARGETDIR}/${NAME} PROPERTY OUTPUT_NAME ${NAME})
  SET_PROPERTY(TARGET ${TARGETDIR}/${NAME} PROPERTY LINKER_LANGUAGE CXX)
  TARGET_LINK_LIBRARIES(${TARGETDIR}/${NAME} ${ARGN} ${THEIA_LIBRARY_DEPENDENCIES})
ENDMACRO (HEADER_LIBRARY)

# Macro for CC Executables/Binaries
MACRO (CC_BINARY NAME)
  ADD_EXECUTABLE(${NAME} ${NAME}.cc)
  TARGET_LINK_LIBRARIES(${NAME} ${ARGN} ${THEIA_LIBRARY_DEPENDENCIES})
ENDMACRO (CC_BINARY)

# Function to generate Protocol buffers in the same dir that the source is in.
function(THEIA_PROTOBUF_GENERATE_CPP SRCS HDRS)
   if(NOT ARGN)
    message(SEND_ERROR "Error: THEIA_PROTOBUF_GENERATE_CPP() called without any proto files")
    return()
  endif()

  # Add any predefined dirs to search for other protos.
  if(DEFINED PROTOBUF_IMPORT_DIRS)
    foreach(DIR ${PROTOBUF_IMPORT_DIRS})
      get_filename_component(ABS_PATH ${DIR} ABSOLUTE)
      list(FIND _protobuf_include_path ${ABS_PATH} _contains_already)
      if(${_contains_already} EQUAL -1)
          list(APPEND _protobuf_include_path -I${ABS_PATH})
      endif()
    endforeach()
  endif()

  set(${SRCS})
  set(${HDRS})
  foreach(FIL ${ARGN})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(FIL_WE ${FIL} NAME_WE)
    get_filename_component(FIL_PATH ${ABS_FIL} PATH)
    list(APPEND ${SRCS} "${FIL_PATH}/${FIL_WE}.pb.cc")
    list(APPEND ${HDRS} "${FIL_PATH}/${FIL_WE}.pb.h")
    add_custom_command(
      OUTPUT "${FIL_PATH}/${FIL_WE}.pb.cc"
             "${FIL_PATH}/${FIL_WE}.pb.h"
      COMMAND  ${PROTOBUF_PROTOC_EXECUTABLE}
      ARGS --cpp_out ${CMAKE_SOURCE_DIR} ${_protobuf_include_path} ${ABS_FIL}
      DEPENDS ${ABS_FIL}
      COMMENT "Running C++ protocol buffer compiler on ${FIL}"
      VERBATIM )
  endforeach()

  set_source_files_properties(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)
  set(${SRCS} ${${SRCS}} PARENT_SCOPE)
  set(${HDRS} ${${HDRS}} PARENT_SCOPE)
endfunction()

# Macro for protocol buffers
MACRO(PROTO_LIBRARY NAME SRCS)
  # If protobufs are enabled, compile them here.
  If (${PROTOBUF_FOUND})
    # Generate cpp files.
    THEIA_PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${SRCS})
    get_filename_component(TARGETFILE ${PROTO_SRCS} NAME)
    CC_LIBRARY(${NAME} ${TARGETFILE} ${ARGN})
  # Otherwise, generate a dummy archive so that it will create a target without
  # consequence.
  ELSE (${PROTOBUF_FOUND})
    HEADER_LIBRARY(${NAME} dummy.h)
  ENDIF (${PROTOBUF_FOUND})
ENDMACRO(PROTO_LIBRARY)

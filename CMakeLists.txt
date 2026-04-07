cmake_minimum_required(VERSION 3.16...3.27)
project(SS_V0_1_1 LANGUAGES CXX)

option(WITH_GEANT4_UIVIS "Build with UI/Vis" ON)

if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()

include(${Geant4_USE_FILE})

# ======================================================
# 🔴 核心：自动包含 include/ 下所有子文件夹
# 用 */ 匹配所有一级子目录，再也不用手动写！
# ======================================================
file(GLOB INCLUDE_SUBDIRS RELATIVE ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/include/*)
foreach(DIR ${INCLUDE_SUBDIRS})
  include_directories(${PROJECT_SOURCE_DIR}/include/${DIR})
endforeach()

# 全局 include
include_directories(${PROJECT_SOURCE_DIR}/include)

# ======================================================
# 🔴 自动收集所有 src/*/*.cc
# ======================================================
file(GLOB_RECURSE SS_SOURCES
  ${PROJECT_SOURCE_DIR}/src/*.cc
  ${PROJECT_SOURCE_DIR}/src/*/*.cc
)

# ======================================================
# 可执行文件
# ======================================================
add_executable(SS_V0_1_1 SS_V0_1_1.cc ${SS_SOURCES})

target_compile_features(SS_V0_1_1 PRIVATE cxx_std_17)
target_link_libraries(SS_V0_1_1 PRIVATE ${Geant4_LIBRARIES})

# 复制宏文件
foreach(_dir macros data)
  if(EXISTS ${PROJECT_SOURCE_DIR}/${_dir})
    add_custom_target(copy_${_dir} ALL
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/${_dir} ${PROJECT_BINARY_DIR}/${_dir}
    )
    add_dependencies(SS_V0_1_1 copy_${_dir})
  endif()
endforeach()

install(TARGETS SS_V0_1_1 DESTINATION bin)
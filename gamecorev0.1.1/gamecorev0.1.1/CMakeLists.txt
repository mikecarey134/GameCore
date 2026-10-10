cmake_minimum_required(VERSION 3.16)
project(gamecore CXX C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

set(THIRD_PARTY "${CMAKE_CURRENT_SOURCE_DIR}/third_party")
set(IRRKLANG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/compat/irrKlang" CACHE PATH
    "irrKlang headers; the default is a silent stand-in")
set(IRRKLANG_LIBRARY "" CACHE FILEPATH "irrKlang library; leave empty with the silent stand-in")

if(MSVC)
  set(QUIET_THIRD_PARTY /w)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(QUIET_THIRD_PARTY -w -Wno-c++11-narrowing)
else()
  set(QUIET_THIRD_PARTY -w -Wno-narrowing)
endif()

find_package(irrlicht CONFIG QUIET)
find_path(IRRLICHT_INCLUDE_DIR irrlicht.h PATH_SUFFIXES irrlicht)
if(TARGET Irrlicht::Irrlicht)
  set(IRRLICHT_LINK Irrlicht::Irrlicht)
else()
  find_library(IRRLICHT_LIBRARY NAMES Irrlicht)
  set(IRRLICHT_LINK "${IRRLICHT_LIBRARY}")
endif()
if(NOT IRRLICHT_INCLUDE_DIR OR NOT IRRLICHT_LINK)
  message(FATAL_ERROR "Irrlicht 1.8 not found. Install it (apt libirrlicht-dev, brew irrlicht, "
    "vcpkg irrlicht) or set IRRLICHT_INCLUDE_DIR and IRRLICHT_LIBRARY.")
endif()
add_library(irrlicht_engine INTERFACE)
target_include_directories(irrlicht_engine INTERFACE "${IRRLICHT_INCLUDE_DIR}")
target_link_libraries(irrlicht_engine INTERFACE ${IRRLICHT_LINK})

set(BULLET_ROOT "${THIRD_PARTY}/bullet-2.77")
file(GLOB_RECURSE BULLET_SOURCES
  "${BULLET_ROOT}/Bullet/LinearMath/*.cpp"
  "${BULLET_ROOT}/Bullet/BulletCollision/*.cpp"
  "${BULLET_ROOT}/Bullet/BulletDynamics/*.cpp"
  "${BULLET_ROOT}/Bullet/BulletSoftBody/*.cpp")
add_library(bullet STATIC ${BULLET_SOURCES})
target_include_directories(bullet SYSTEM PUBLIC "${BULLET_ROOT}" "${BULLET_ROOT}/Bullet")
target_compile_options(bullet PRIVATE ${QUIET_THIRD_PARTY})

set(IRRBULLET_COPY "${CMAKE_CURRENT_BINARY_DIR}/irrbullet")
file(COPY "${THIRD_PARTY}/irrbullet-0.1.8/" DESTINATION "${IRRBULLET_COPY}"
     FILES_MATCHING PATTERN "*.h" PATTERN "*.cpp")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/kinematiccharactercontroller.h"
               "${IRRBULLET_COPY}/kinematiccharactercontroller.h" COPYONLY)
if(NOT EXISTS "${IRRBULLET_COPY}/irrBullet.h")
  file(WRITE "${IRRBULLET_COPY}/irrBullet.h" "#include \"irrbullet.h\"\n")
endif()
file(GLOB IRRBULLET_SOURCES "${IRRBULLET_COPY}/*.cpp")
list(REMOVE_ITEM IRRBULLET_SOURCES "${IRRBULLET_COPY}/kinematiccharactercontroller.cpp")
add_library(irrbullet STATIC ${IRRBULLET_SOURCES})
target_include_directories(irrbullet SYSTEM PUBLIC "${IRRBULLET_COPY}")
target_link_libraries(irrbullet PUBLIC bullet irrlicht_engine)
target_compile_options(irrbullet PRIVATE ${QUIET_THIRD_PARTY})

file(GLOB RAKNET_SOURCES "${THIRD_PARTY}/raknet/Source/*.cpp")
add_library(raknet STATIC ${RAKNET_SOURCES})
target_include_directories(raknet SYSTEM PUBLIC "${THIRD_PARTY}/raknet/Source")
target_compile_options(raknet PRIVATE ${QUIET_THIRD_PARTY})
find_package(Threads REQUIRED)
target_link_libraries(raknet PUBLIC Threads::Threads)
if(WIN32)
  target_link_libraries(raknet PUBLIC ws2_32)
endif()

file(GLOB LUA_SOURCES "${THIRD_PARTY}/lua-5.1.5/src/*.c")
list(REMOVE_ITEM LUA_SOURCES
  "${THIRD_PARTY}/lua-5.1.5/src/lua.c"
  "${THIRD_PARTY}/lua-5.1.5/src/luac.c"
  "${THIRD_PARTY}/lua-5.1.5/src/print.c")
add_library(lua51 STATIC ${LUA_SOURCES})
target_include_directories(lua51 SYSTEM PUBLIC "${THIRD_PARTY}/lua-5.1.5/src")
target_compile_options(lua51 PRIVATE ${QUIET_THIRD_PARTY})

add_library(gamecore_inventory STATIC inventory/Catalog.cpp inventory/Inventory.cpp)
target_include_directories(gamecore_inventory PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/inventory")

add_library(gamecore_inventory_ui STATIC inventory_ui/InventoryLayout.cpp inventory_ui/InventoryController.cpp)
target_include_directories(gamecore_inventory_ui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/inventory_ui")
target_link_libraries(gamecore_inventory_ui PUBLIC gamecore_inventory)

enable_testing()
add_executable(inventory_ui_tests inventory_ui/tests/main.cpp inventory_ui/tests/layout_test.cpp inventory_ui/tests/controller_test.cpp)
target_include_directories(inventory_ui_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/inventory/third_party/doctest")
target_link_libraries(inventory_ui_tests PRIVATE gamecore_inventory_ui)
add_test(NAME inventory_ui_tests COMMAND inventory_ui_tests)

foreach(target gamecore_inventory gamecore_inventory_ui inventory_ui_tests)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -pedantic)
  endif()
  set_target_properties(${target} PROPERTIES COMPILE_WARNING_AS_ERROR ON)
endforeach()

set(GAMECORE_SOURCES
  ChatQueue.cpp ClueObject.cpp Config_Reader.cpp console.cpp consoleevent.cpp
  defaultCmds.cpp exampleframework.cpp GrassNode.cpp GUI.cpp InternalServer.cpp
  kinematiccharactercontroller.cpp main.cpp mapLoader.cpp NetworkData.cpp NPC.cpp
  ParticleSystem.cpp player.cpp remotePlayer.cpp SolarCycle.cpp utils.cpp
  WeaponObject.cpp)

add_executable(gamecore ${GAMECORE_SOURCES})
target_include_directories(gamecore PRIVATE "${IRRKLANG_INCLUDE_DIR}")
target_link_libraries(gamecore PRIVATE irrbullet raknet lua51 irrlicht_engine ${IRRKLANG_LIBRARY} ${CMAKE_DL_LIBS})

if(WIN32)
  target_link_libraries(gamecore PRIVATE ws2_32 opengl32)
  if(MSVC)
    target_link_options(gamecore PRIVATE
      /NODEFAULTLIB:irrKlang.lib /NODEFAULTLIB:Irrlicht.lib /NODEFAULTLIB:IrrConsole-DEBUG.lib)
  endif()
elseif(APPLE)
  target_link_libraries(gamecore PRIVATE "-framework Cocoa" "-framework OpenGL" "-framework IOKit")
else()
  find_package(OpenGL REQUIRED)
  find_package(X11 REQUIRED)
  target_link_libraries(gamecore PRIVATE OpenGL::GL X11::X11 X11::Xext)
  if(TARGET X11::Xxf86vm)
    target_link_libraries(gamecore PRIVATE X11::Xxf86vm)
  endif()
endif()

set_target_properties(gamecore PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

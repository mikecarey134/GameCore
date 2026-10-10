cmake_minimum_required(VERSION 3.16)
project(gamecore CXX C)

# Third-party locations. Each defaults to the layout of ~/src/gamecore-deps;
# override with -D<NAME>=<path> when configuring.
set(DEPS_DIR "$ENV{HOME}/src/gamecore-deps" CACHE PATH "Root of the third-party checkouts")
set(IRRLICHT_DIR "$ENV{HOME}/src/irrlicht-1.8.5" CACHE PATH "Irrlicht 1.8.x SDK (built)")
set(IRRBULLET_DIR "${DEPS_DIR}/irrBullet-0.1.8/irrBulletSDK-0.1.8/source" CACHE PATH "irrBullet 0.1.8 source")
set(BULLET_PREFIX "${DEPS_DIR}/prefix" CACHE PATH "Bullet 2.77 and Lua 5.1 install prefix")
set(RAKNET_DIR "${DEPS_DIR}/RakNet" CACHE PATH "RakNet source")
set(RAKNET_LIB "${DEPS_DIR}/build-raknet/Lib/LibStatic/libRakNetLibStatic.a" CACHE FILEPATH "RakNet static library")
set(IRRKLANG_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/compat/irrKlang" CACHE PATH
    "irrKlang headers; the default is a silent stand-in")
set(IRRKLANG_LIB "" CACHE FILEPATH "irrKlang library; leave empty with the silent stand-in")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

# GameCore carries its own version of irrBullet's character controller, and
# irrbullet.h includes that header by a path relative to itself. Build
# irrBullet from a copy with GameCore's header laid over the stock one so
# every file sees the same class.
set(IRRBULLET_COPY "${CMAKE_BINARY_DIR}/irrbullet")
file(COPY "${IRRBULLET_DIR}/" DESTINATION "${IRRBULLET_COPY}"
     PATTERN "DeprecatedCancelled" EXCLUDE PATTERN "msvc" EXCLUDE)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/kinematiccharactercontroller.h"
               "${IRRBULLET_COPY}/kinematiccharactercontroller.h" COPYONLY)
# Windows code spells it irrBullet.h; the file is irrbullet.h.
file(WRITE "${IRRBULLET_COPY}/irrBullet.h" "#include \"irrbullet.h\"\n")
file(GLOB IRRBULLET_SOURCES "${IRRBULLET_COPY}/*.cpp")
list(REMOVE_ITEM IRRBULLET_SOURCES "${IRRBULLET_COPY}/kinematiccharactercontroller.cpp")

add_library(irrbullet STATIC ${IRRBULLET_SOURCES})
target_include_directories(irrbullet PUBLIC
  "${IRRBULLET_COPY}"
  "${IRRBULLET_COPY}/bheaders"
  "${IRRBULLET_COPY}/bheaders/Bullet"
  "${IRRLICHT_DIR}/include")
target_compile_options(irrbullet PRIVATE -w)

set(GAMECORE_SOURCES
  ChatQueue.cpp ClueObject.cpp Config_Reader.cpp console.cpp consoleevent.cpp
  defaultCmds.cpp exampleframework.cpp GrassNode.cpp GUI.cpp InternalServer.cpp
  kinematiccharactercontroller.cpp main.cpp mapLoader.cpp NetworkData.cpp NPC.cpp
  ParticleSystem.cpp player.cpp remotePlayer.cpp SolarCycle.cpp utils.cpp
  WeaponObject.cpp)

add_executable(gamecore ${GAMECORE_SOURCES})
target_include_directories(gamecore PRIVATE
  "${IRRKLANG_INCLUDE_DIR}"
  "${RAKNET_DIR}/Source"
  "${BULLET_PREFIX}/include/lua51")
target_link_libraries(gamecore PRIVATE
  irrbullet
  "${BULLET_PREFIX}/lib/libBulletSoftBody.a"
  "${BULLET_PREFIX}/lib/libBulletDynamics.a"
  "${BULLET_PREFIX}/lib/libBulletCollision.a"
  "${BULLET_PREFIX}/lib/libLinearMath.a"
  "${RAKNET_LIB}"
  "${BULLET_PREFIX}/lib/liblua51.a"
  "${IRRLICHT_DIR}/lib/Linux/libIrrlicht.a"
  ${IRRKLANG_LIB}
  GL X11 Xext pthread dl m)

# Run from the source directory: the game loads config/, models/ and the
# rest by relative path.
set_target_properties(gamecore PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

cmake_minimum_required(VERSION 3.16)
project(gamecore CXX C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

set(THIRD_PARTY "${CMAKE_CURRENT_SOURCE_DIR}/third_party")

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
target_include_directories(irrlicht_engine SYSTEM INTERFACE "${IRRLICHT_INCLUDE_DIR}")
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
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/engine/kinematiccharactercontroller.h"
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

set(ENET_ROOT "${THIRD_PARTY}/enet-1.3.18")
file(GLOB ENET_SOURCES "${ENET_ROOT}/*.c")
add_library(enet STATIC ${ENET_SOURCES})
target_include_directories(enet SYSTEM PUBLIC "${ENET_ROOT}/include")
target_compile_options(enet PRIVATE ${QUIET_THIRD_PARTY})
if(WIN32)
  target_link_libraries(enet PUBLIC ws2_32 winmm)
else()
  include(CheckFunctionExists)
  include(CheckStructHasMember)
  include(CheckTypeSize)
  foreach(function fcntl poll getaddrinfo getnameinfo gethostbyname_r gethostbyaddr_r inet_pton inet_ntop)
    string(TOUPPER "HAS_${function}" flag)
    check_function_exists(${function} ${flag})
    if(${flag})
      target_compile_definitions(enet PRIVATE ${flag}=1)
    endif()
  endforeach()
  check_struct_has_member("struct msghdr" "msg_flags" "sys/types.h;sys/socket.h" HAS_MSGHDR_FLAGS)
  if(HAS_MSGHDR_FLAGS)
    target_compile_definitions(enet PRIVATE HAS_MSGHDR_FLAGS=1)
  endif()
  set(CMAKE_EXTRA_INCLUDE_FILES "sys/types.h;sys/socket.h")
  check_type_size("socklen_t" HAS_SOCKLEN_T BUILTIN_TYPES_ONLY)
  unset(CMAKE_EXTRA_INCLUDE_FILES)
  if(HAS_SOCKLEN_T)
    target_compile_definitions(enet PRIVATE HAS_SOCKLEN_T=1)
  endif()
endif()

add_library(miniaudio STATIC "${THIRD_PARTY}/miniaudio-0.11.25/miniaudio.c")
target_include_directories(miniaudio SYSTEM PUBLIC "${THIRD_PARTY}/miniaudio-0.11.25")
target_compile_options(miniaudio PRIVATE ${QUIET_THIRD_PARTY})
target_link_libraries(miniaudio PUBLIC Threads::Threads ${CMAKE_DL_LIBS})
if(UNIX AND NOT APPLE)
  target_link_libraries(miniaudio PUBLIC m)
endif()

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

set(ENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/engine")
add_library(gamecore_engine STATIC
  ${ENGINE_DIR}/kinematiccharactercontroller.cpp ${ENGINE_DIR}/kinematiccharactercontroller.h
  ${ENGINE_DIR}/ChatQueue.cpp ${ENGINE_DIR}/ChatQueue.h
  ${ENGINE_DIR}/ParticleSystem.cpp ${ENGINE_DIR}/ParticleSystem.h
  ${ENGINE_DIR}/Config_Reader.cpp ${ENGINE_DIR}/Config_Reader.h
  ${ENGINE_DIR}/InteractiveObject.h ${ENGINE_DIR}/SAppcontext.h
  ${ENGINE_DIR}/SoundEngine.cpp ${ENGINE_DIR}/SoundEngine.h
  ${ENGINE_DIR}/mapLoader.cpp ${ENGINE_DIR}/mapLoader.h
  ${ENGINE_DIR}/OrbitCamera.cpp ${ENGINE_DIR}/OrbitCamera.h
  ${ENGINE_DIR}/MouseCapture.cpp ${ENGINE_DIR}/MouseCapture.h
  ${ENGINE_DIR}/InputRouter.cpp ${ENGINE_DIR}/InputRouter.h ${ENGINE_DIR}/InputState.h
  ${ENGINE_DIR}/console.cpp ${ENGINE_DIR}/console.h ${ENGINE_DIR}/consoleCommands.cpp ${ENGINE_DIR}/consoleCommands.h
  ${ENGINE_DIR}/utils.cpp ${ENGINE_DIR}/utils.h ${ENGINE_DIR}/includes.h
  ${ENGINE_DIR}/ConsoleLayer.cpp ${ENGINE_DIR}/ConsoleLayer.h
  ${ENGINE_DIR}/LuaVM.cpp ${ENGINE_DIR}/LuaVM.h
  ${ENGINE_DIR}/DebugOverlay.cpp ${ENGINE_DIR}/DebugOverlay.h
  ${ENGINE_DIR}/ActionMap.cpp ${ENGINE_DIR}/ActionMap.h
  ${ENGINE_DIR}/UiAnchor.cpp ${ENGINE_DIR}/UiAnchor.h)
target_include_directories(gamecore_engine PUBLIC "${ENGINE_DIR}")
target_link_libraries(gamecore_engine PUBLIC irrbullet irrlicht_engine gamecore_net PRIVATE miniaudio lua51)

add_library(gamecore_net STATIC
  ${ENGINE_DIR}/net/ByteStream.cpp ${ENGINE_DIR}/net/ByteStream.h
  ${ENGINE_DIR}/net/MessageFrame.cpp ${ENGINE_DIR}/net/MessageFrame.h
  ${ENGINE_DIR}/net/MessageRouter.cpp ${ENGINE_DIR}/net/MessageRouter.h ${ENGINE_DIR}/net/Transport.h
  ${ENGINE_DIR}/net/LoopbackTransport.cpp ${ENGINE_DIR}/net/LoopbackTransport.h
  ${ENGINE_DIR}/net/EnetTransport.cpp ${ENGINE_DIR}/net/EnetTransport.h)
target_include_directories(gamecore_net PUBLIC "${ENGINE_DIR}")
target_link_libraries(gamecore_net PRIVATE enet)

set(GAMECORE_SOURCES
  ClueObject.cpp GameFlow.cpp
  defaultCmds.cpp exampleframework.cpp GrassNode.cpp InternalServer.cpp
  main.cpp GameMap.cpp GameLayers.cpp Screens.cpp NetworkData.cpp NPC.cpp
  player.cpp remotePlayer.cpp SolarCycle.cpp
  WeaponObject.cpp inventory_ui/InventoryScreen.cpp)

add_executable(gamecore ${GAMECORE_SOURCES})
target_include_directories(gamecore PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(gamecore PRIVATE gamecore_engine)

add_executable(engine_tests engine/tests/main.cpp engine/tests/orbit_camera_test.cpp engine/tests/input_router_test.cpp
  engine/tests/lua_vm_test.cpp engine/tests/action_map_test.cpp
  engine/tests/ui_anchor_test.cpp engine/tests/window_size_test.cpp
  engine/tests/byte_stream_test.cpp engine/tests/message_frame_test.cpp
  engine/tests/message_router_test.cpp engine/tests/loopback_transport_test.cpp
  engine/tests/enet_transport_test.cpp ${ENGINE_DIR}/OrbitCamera.cpp ${ENGINE_DIR}/InputRouter.cpp
  ${ENGINE_DIR}/LuaVM.cpp ${ENGINE_DIR}/ActionMap.cpp ${ENGINE_DIR}/UiAnchor.cpp)
target_include_directories(engine_tests PRIVATE "${ENGINE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/inventory/third_party/doctest")
target_link_libraries(engine_tests PRIVATE irrlicht_engine lua51 gamecore_net)
add_test(NAME engine_tests COMMAND engine_tests)

add_library(game_protocol STATIC protocol/GameMessages.cpp protocol/GameMessages.h)
target_include_directories(game_protocol PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(game_protocol PUBLIC gamecore_net)

add_library(gamecore_server_core STATIC server/GameServer.cpp server/GameServer.h)
target_link_libraries(gamecore_server_core PUBLIC game_protocol)

add_executable(server_tests server/tests/main.cpp server/tests/protocol_test.cpp server/tests/game_server_test.cpp)
target_include_directories(server_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/inventory/third_party/doctest")
target_link_libraries(server_tests PRIVATE gamecore_server_core)
add_test(NAME server_tests COMMAND server_tests)

foreach(target gamecore_engine gamecore engine_tests gamecore_net game_protocol gamecore_server_core server_tests)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4)
    target_compile_definitions(${target} PRIVATE _CRT_SECURE_NO_WARNINGS)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -pedantic)
  endif()
  if(APPLE)
    target_compile_definitions(${target} PRIVATE GL_SILENCE_DEPRECATION)
  endif()
  set_target_properties(${target} PROPERTIES COMPILE_WARNING_AS_ERROR ON)
endforeach()
target_link_libraries(gamecore PRIVATE gamecore_inventory_ui irrbullet raknet irrlicht_engine ${CMAKE_DL_LIBS})

if(WIN32)
  target_link_libraries(gamecore PRIVATE ws2_32 opengl32)
  if(MSVC)
    target_link_options(gamecore PRIVATE
      /NODEFAULTLIB:Irrlicht.lib /NODEFAULTLIB:IrrConsole-DEBUG.lib)
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

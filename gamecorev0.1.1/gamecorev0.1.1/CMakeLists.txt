cmake_minimum_required(VERSION 3.16)
project(gamecore CXX C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

set(THIRD_PARTY "${CMAKE_CURRENT_SOURCE_DIR}/third_party")
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(GAMECORE_TOP_LEVEL ON)
else()
  set(GAMECORE_TOP_LEVEL OFF)
endif()

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
if(WIN32)
  target_link_libraries(irrlicht_engine INTERFACE ws2_32 opengl32)
elseif(APPLE)
  target_link_libraries(irrlicht_engine INTERFACE "-framework Cocoa" "-framework OpenGL" "-framework IOKit")
else()
  find_package(OpenGL REQUIRED)
  find_package(X11 REQUIRED)
  target_link_libraries(irrlicht_engine INTERFACE OpenGL::GL X11::X11 X11::Xext)
  if(TARGET X11::Xxf86vm)
    target_link_libraries(irrlicht_engine INTERFACE X11::Xxf86vm)
  endif()
endif()

set(JOLT_ROOT "${THIRD_PARTY}/jolt-5.6.0")
file(GLOB_RECURSE JOLT_SOURCES "${JOLT_ROOT}/Jolt/*.cpp")
list(FILTER JOLT_SOURCES EXCLUDE REGEX "/Jolt/(Compute/(CPU|DX12|VK|MTL)|Shaders)/")
add_library(jolt STATIC ${JOLT_SOURCES})
set_target_properties(jolt PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF)
target_include_directories(jolt SYSTEM PUBLIC "${JOLT_ROOT}")
target_compile_definitions(jolt PUBLIC JPH_DEBUG_RENDERER)
target_compile_options(jolt PRIVATE ${QUIET_THIRD_PARTY})
if(MSVC)
  target_compile_options(jolt PUBLIC /GR-)
else()
  target_compile_options(jolt PUBLIC -fno-rtti -fno-exceptions)
endif()
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64)$" AND NOT CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
  target_compile_definitions(jolt PUBLIC JPH_USE_SSE4_1 JPH_USE_SSE4_2)
  if(NOT MSVC)
    target_compile_options(jolt PUBLIC -msse4.2 -mpopcnt)
  endif()
endif()

find_package(Threads REQUIRED)

set(LUAJIT_ROOT "${THIRD_PARTY}/luajit")
set(LUAJIT_SRC "${LUAJIT_ROOT}/src")
set(LUAJIT_GEN "${CMAKE_CURRENT_BINARY_DIR}/luajit")
file(MAKE_DIRECTORY "${LUAJIT_GEN}")
set(LUAJIT_LIBS lib_base.c lib_math.c lib_bit.c lib_string.c lib_table.c lib_io.c lib_os.c
  lib_package.c lib_debug.c lib_jit.c lib_ffi.c lib_buffer.c)

if(MSVC)
  set(LUAJIT_DASM_FLAGS -D WIN -D JIT -D FFI -D ENDIAN_LE -D FPU -D P64)
  set(LUAJIT_DASC vm_x64.dasc)
  set(LUAJIT_VM_MODE peobj)
  set(LUAJIT_VM "${LUAJIT_GEN}/lj_vm.obj")
else()
  enable_language(ASM)
  execute_process(COMMAND ${CMAKE_C_COMPILER} -E -dM -x c "${LUAJIT_SRC}/lj_arch.h"
    OUTPUT_VARIABLE LUAJIT_ARCH RESULT_VARIABLE LUAJIT_ARCH_RESULT)
  if(NOT LUAJIT_ARCH_RESULT EQUAL 0)
    message(FATAL_ERROR "Could not preprocess LuaJIT's lj_arch.h")
  endif()
  set(LUAJIT_DASM_FLAGS "")
  foreach(pair "LJ_LE 1:ENDIAN_LE" "LJ_BE 1:ENDIAN_BE" "LJ_ARCH_BITS 64:P64" "LJ_HASJIT 1:JIT"
      "LJ_HASFFI 1:FFI" "LJ_DUALNUM 1:DUALNUM" "LJ_ARCH_HASFPU 1:FPU" "LJ_ABI_SOFTFP 0:HFABI"
      "LJ_NO_UNWIND 1:NO_UNWIND" "LJ_ABI_PAUTH 1:PAUTH" "LJ_ABI_BRANCH_TRACK 1:BRANCH_TRACK"
      "LJ_ABI_SHADOW_STACK 1:SHADOW_STACK")
    string(REPLACE ":" ";" parts "${pair}")
    list(GET parts 0 define)
    list(GET parts 1 flag)
    if(LUAJIT_ARCH MATCHES "#define ${define}\n")
      list(APPEND LUAJIT_DASM_FLAGS -D ${flag})
    endif()
  endforeach()
  if(LUAJIT_ARCH MATCHES "#define LJ_ARCH_VERSION ([0-9]+)")
    list(APPEND LUAJIT_DASM_FLAGS -D VER=${CMAKE_MATCH_1})
  else()
    list(APPEND LUAJIT_DASM_FLAGS -D VER=)
  endif()
  if(LUAJIT_ARCH MATCHES "#define LJ_TARGET_X64 1")
    set(LUAJIT_DASC vm_x64.dasc)
  elseif(LUAJIT_ARCH MATCHES "#define LJ_TARGET_ARM64 1")
    set(LUAJIT_DASC vm_arm64.dasc)
  else()
    message(FATAL_ERROR "LuaJIT is only set up here for x64 and arm64")
  endif()
  if(APPLE)
    set(LUAJIT_VM_MODE machasm)
  else()
    set(LUAJIT_VM_MODE elfasm)
  endif()
  set(LUAJIT_VM "${LUAJIT_GEN}/lj_vm.S")
endif()

add_executable(luajit_minilua "${LUAJIT_SRC}/host/minilua.c")
target_compile_options(luajit_minilua PRIVATE ${QUIET_THIRD_PARTY})
if(NOT WIN32)
  target_link_libraries(luajit_minilua PRIVATE m)
endif()

add_custom_command(OUTPUT "${LUAJIT_GEN}/buildvm_arch.h"
  COMMAND luajit_minilua "${LUAJIT_ROOT}/dynasm/dynasm.lua" ${LUAJIT_DASM_FLAGS}
    -o "${LUAJIT_GEN}/buildvm_arch.h" "${LUAJIT_SRC}/${LUAJIT_DASC}"
  DEPENDS luajit_minilua "${LUAJIT_SRC}/${LUAJIT_DASC}")
add_custom_command(OUTPUT "${LUAJIT_GEN}/luajit.h"
  COMMAND luajit_minilua "${LUAJIT_SRC}/host/genversion.lua" "${LUAJIT_SRC}/luajit_rolling.h"
    "${LUAJIT_ROOT}/.relver" "${LUAJIT_GEN}/luajit.h"
  DEPENDS luajit_minilua "${LUAJIT_SRC}/luajit_rolling.h" "${LUAJIT_ROOT}/.relver")

file(GLOB LUAJIT_BUILDVM_SOURCES "${LUAJIT_SRC}/host/buildvm*.c")
add_executable(luajit_buildvm ${LUAJIT_BUILDVM_SOURCES} "${LUAJIT_GEN}/buildvm_arch.h" "${LUAJIT_GEN}/luajit.h")
target_include_directories(luajit_buildvm PRIVATE "${LUAJIT_GEN}" "${LUAJIT_SRC}" "${LUAJIT_ROOT}/dynasm")
target_compile_options(luajit_buildvm PRIVATE ${QUIET_THIRD_PARTY})

set(LUAJIT_GENERATED "${LUAJIT_VM}")
add_custom_command(OUTPUT "${LUAJIT_VM}"
  COMMAND luajit_buildvm -m ${LUAJIT_VM_MODE} -o "${LUAJIT_VM}"
  DEPENDS luajit_buildvm)
foreach(mode bcdef ffdef libdef recdef)
  add_custom_command(OUTPUT "${LUAJIT_GEN}/lj_${mode}.h"
    COMMAND luajit_buildvm -m ${mode} -o "${LUAJIT_GEN}/lj_${mode}.h" ${LUAJIT_LIBS}
    WORKING_DIRECTORY "${LUAJIT_SRC}"
    DEPENDS luajit_buildvm)
  list(APPEND LUAJIT_GENERATED "${LUAJIT_GEN}/lj_${mode}.h")
endforeach()
add_custom_command(OUTPUT "${LUAJIT_GEN}/lj_folddef.h"
  COMMAND luajit_buildvm -m folddef -o "${LUAJIT_GEN}/lj_folddef.h" lj_opt_fold.c
  WORKING_DIRECTORY "${LUAJIT_SRC}"
  DEPENDS luajit_buildvm "${LUAJIT_SRC}/lj_opt_fold.c")
list(APPEND LUAJIT_GENERATED "${LUAJIT_GEN}/lj_folddef.h" "${LUAJIT_GEN}/luajit.h")

add_library(luajit STATIC "${LUAJIT_SRC}/ljamalg.c" ${LUAJIT_GENERATED})
if(MSVC)
  set_source_files_properties("${LUAJIT_VM}" PROPERTIES EXTERNAL_OBJECT TRUE GENERATED TRUE)
  target_compile_definitions(luajit PRIVATE _CRT_SECURE_NO_DEPRECATE)
else()
  target_compile_definitions(luajit PRIVATE _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE LUAJIT_UNWIND_EXTERNAL)
  target_compile_options(luajit PRIVATE -U_FORTIFY_SOURCE)
  target_link_libraries(luajit PUBLIC m ${CMAKE_DL_LIBS})
endif()
target_include_directories(luajit SYSTEM PUBLIC "${LUAJIT_SRC}" "${LUAJIT_GEN}")
target_compile_options(luajit PRIVATE ${QUIET_THIRD_PARTY})

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

set(ENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/engine")
add_library(gamecore_engine STATIC
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
  ${ENGINE_DIR}/UiAnchor.cpp ${ENGINE_DIR}/UiAnchor.h
  ${ENGINE_DIR}/KeyValueFile.cpp ${ENGINE_DIR}/KeyValueFile.h
  ${ENGINE_DIR}/SafeFileName.cpp ${ENGINE_DIR}/SafeFileName.h ${ENGINE_DIR}/ScreenStack.h
  ${ENGINE_DIR}/GuiInputLayer.cpp ${ENGINE_DIR}/GuiInputLayer.h ${ENGINE_DIR}/GuiPanel.cpp ${ENGINE_DIR}/GuiPanel.h
  ${ENGINE_DIR}/Canvas.h ${ENGINE_DIR}/IrrlichtCanvas.cpp ${ENGINE_DIR}/IrrlichtCanvas.h)
target_include_directories(gamecore_engine PUBLIC "${ENGINE_DIR}")
target_link_libraries(gamecore_engine PUBLIC irrlicht_engine gamecore_net gamecore_physics PRIVATE miniaudio luajit)

add_library(gamecore_physics STATIC
  ${ENGINE_DIR}/physics/PhysicsWorld.h ${ENGINE_DIR}/physics/CharacterController.h
  ${ENGINE_DIR}/physics/JoltBackend.cpp)
set_target_properties(gamecore_physics PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF)
target_include_directories(gamecore_physics PUBLIC "${ENGINE_DIR}")
target_link_libraries(gamecore_physics PUBLIC irrlicht_engine PRIVATE jolt)

add_library(gamecore_net STATIC
  ${ENGINE_DIR}/net/ByteStream.cpp ${ENGINE_DIR}/net/ByteStream.h
  ${ENGINE_DIR}/net/MessageFrame.cpp ${ENGINE_DIR}/net/MessageFrame.h
  ${ENGINE_DIR}/net/MessageRouter.cpp ${ENGINE_DIR}/net/MessageRouter.h ${ENGINE_DIR}/net/Transport.h
  ${ENGINE_DIR}/net/LoopbackTransport.cpp ${ENGINE_DIR}/net/LoopbackTransport.h
  ${ENGINE_DIR}/net/EnetTransport.cpp ${ENGINE_DIR}/net/EnetTransport.h
  ${ENGINE_DIR}/net/Interpolator.cpp ${ENGINE_DIR}/net/Interpolator.h)
target_include_directories(gamecore_net PUBLIC "${ENGINE_DIR}")
target_link_libraries(gamecore_net PRIVATE enet)

add_library(game_protocol STATIC protocol/GameMessages.cpp protocol/GameMessages.h)
target_include_directories(game_protocol PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(game_protocol PUBLIC gamecore_net)

add_library(gamecore_server_core STATIC server/GameServer.cpp server/GameServer.h
  server/ServerConsole.cpp server/ServerConsole.h)
target_link_libraries(gamecore_server_core PUBLIC game_protocol)

add_library(game_client_net STATIC client/NetClient.cpp client/NetClient.h)
target_link_libraries(game_client_net PUBLIC game_protocol)

set(GAMECORE_LIBRARIES gamecore_engine gamecore_net gamecore_physics game_protocol gamecore_server_core game_client_net)
set(GAMECORE_STRICT_TARGETS ${GAMECORE_LIBRARIES})

if(GAMECORE_TOP_LEVEL)
  enable_testing()
  add_executable(inventory_ui_tests inventory_ui/tests/main.cpp inventory_ui/tests/layout_test.cpp inventory_ui/tests/controller_test.cpp)
  target_include_directories(inventory_ui_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/inventory/third_party/doctest")
  target_link_libraries(inventory_ui_tests PRIVATE gamecore_inventory_ui)
  add_test(NAME inventory_ui_tests COMMAND inventory_ui_tests)

  set(GAMECORE_SOURCES
    ClueObject.cpp GameFlow.cpp
    defaultCmds.cpp GrassNode.cpp InternalServer.cpp
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
    engine/tests/enet_transport_test.cpp engine/tests/interpolator_test.cpp engine/tests/canvas_test.cpp engine/tests/mouse_capture_test.cpp
    engine/tests/key_value_file_test.cpp engine/tests/safe_file_name_test.cpp engine/tests/screen_stack_test.cpp engine/tests/gui_test.cpp
    ${ENGINE_DIR}/KeyValueFile.cpp ${ENGINE_DIR}/SafeFileName.cpp ${ENGINE_DIR}/GuiInputLayer.cpp ${ENGINE_DIR}/GuiPanel.cpp ${ENGINE_DIR}/MouseCapture.cpp ${ENGINE_DIR}/OrbitCamera.cpp ${ENGINE_DIR}/InputRouter.cpp
    ${ENGINE_DIR}/LuaVM.cpp ${ENGINE_DIR}/ActionMap.cpp ${ENGINE_DIR}/UiAnchor.cpp
    ${ENGINE_DIR}/IrrlichtCanvas.cpp ${ENGINE_DIR}/RecordingCanvas.cpp)
  target_include_directories(engine_tests PRIVATE "${ENGINE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/inventory/third_party/doctest")
  target_link_libraries(engine_tests PRIVATE irrlicht_engine luajit gamecore_net)
  target_compile_definitions(engine_tests PRIVATE GAMECORE_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
  add_test(NAME engine_tests COMMAND engine_tests)
  add_test(NAME irrlicht_boundary COMMAND ${CMAKE_COMMAND}
    -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -DIRRLICHT_INCLUDE_DIR=${IRRLICHT_INCLUDE_DIR}
    -P ${CMAKE_CURRENT_SOURCE_DIR}/engine/tests/irrlicht_boundary.cmake)

  add_executable(physics_tests engine/tests/main.cpp engine/tests/physics_contract_test.cpp)
  target_include_directories(physics_tests PRIVATE "${ENGINE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/inventory/third_party/doctest")
  target_link_libraries(physics_tests PRIVATE gamecore_physics)
  add_test(NAME physics_tests COMMAND physics_tests)

  add_executable(gamecore_server server/main.cpp)
  target_link_libraries(gamecore_server PRIVATE gamecore_server_core Threads::Threads)

  add_executable(net_tests server/tests/main.cpp server/tests/protocol_test.cpp server/tests/game_server_test.cpp
    server/tests/server_console_test.cpp server/tests/integration_test.cpp client/tests/net_client_test.cpp)
  target_include_directories(net_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/inventory/third_party/doctest")
  target_link_libraries(net_tests PRIVATE gamecore_server_core game_client_net Threads::Threads)
  add_test(NAME net_tests COMMAND net_tests)

  foreach(target gamecore_inventory gamecore_inventory_ui inventory_ui_tests)
    if(MSVC)
      target_compile_options(${target} PRIVATE /W4)
    else()
      target_compile_options(${target} PRIVATE -Wall -Wextra -pedantic)
    endif()
    set_target_properties(${target} PROPERTIES COMPILE_WARNING_AS_ERROR ON)
  endforeach()

  list(APPEND GAMECORE_STRICT_TARGETS gamecore engine_tests physics_tests gamecore_server net_tests)
  target_link_libraries(gamecore PRIVATE gamecore_inventory_ui game_client_net irrlicht_engine ${CMAKE_DL_LIBS})

  if(MSVC)
    target_link_options(gamecore PRIVATE
      /NODEFAULTLIB:Irrlicht.lib /NODEFAULTLIB:IrrConsole-DEBUG.lib)
  endif()

  set_target_properties(gamecore PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

foreach(target ${GAMECORE_STRICT_TARGETS})
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

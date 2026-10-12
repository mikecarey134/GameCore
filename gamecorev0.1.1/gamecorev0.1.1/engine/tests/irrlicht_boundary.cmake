cmake_minimum_required(VERSION 3.16)

set(ALLOWED vector2d.h vector3d.h rect.h dimension2d.h position2d.h matrix4.h quaternion.h
  aabbox3d.h line3d.h triangle3d.h plane3d.h SColor.h irrString.h irrMath.h irrTypes.h
  IEventReceiver.h Keycodes.h)

file(GLOB IRRLICHT_HEADERS RELATIVE "${IRRLICHT_INCLUDE_DIR}" "${IRRLICHT_INCLUDE_DIR}/*.h")
file(GLOB_RECURSE CLEAN RELATIVE "${SOURCE_DIR}"
  "${SOURCE_DIR}/engine/net/*.cpp" "${SOURCE_DIR}/engine/net/*.h"
  "${SOURCE_DIR}/engine/LuaVM*" "${SOURCE_DIR}/engine/SoundEngine*"
  "${SOURCE_DIR}/protocol/*.cpp" "${SOURCE_DIR}/protocol/*.h"
  "${SOURCE_DIR}/server/*.cpp" "${SOURCE_DIR}/server/*.h"
  "${SOURCE_DIR}/client/*.cpp" "${SOURCE_DIR}/client/*.h"
  "${SOURCE_DIR}/inventory/*.cpp" "${SOURCE_DIR}/inventory/*.h"
  "${SOURCE_DIR}/inventory_ui/InventoryController*" "${SOURCE_DIR}/inventory_ui/InventoryLayout*")
list(FILTER CLEAN EXCLUDE REGEX "/third_party/")

list(LENGTH CLEAN count)
if(count LESS 30)
  message(FATAL_ERROR "Only ${count} clean files found; the module globs are wrong")
endif()
list(LENGTH IRRLICHT_HEADERS count)
if(count LESS 100)
  message(FATAL_ERROR "Only ${count} Irrlicht headers found in '${IRRLICHT_INCLUDE_DIR}'")
endif()

set(failures "")
foreach(file ${CLEAN})
  file(STRINGS "${SOURCE_DIR}/${file}" includes REGEX "^[ \t]*#[ \t]*include")
  foreach(line ${includes})
    string(REGEX REPLACE "^[ \t]*#[ \t]*include[ \t]*[<\"]([^>\"]+)[>\"].*" "\\1" header "${line}")
    get_filename_component(name "${header}" NAME)
    if(header MATCHES "irrlicht" OR (name IN_LIST IRRLICHT_HEADERS AND NOT name IN_LIST ALLOWED))
      string(APPEND failures "\n  ${file}: ${line}")
    endif()
  endforeach()
endforeach()

if(failures)
  message(FATAL_ERROR "Irrlicht included in a module that must stay free of it:${failures}")
endif()

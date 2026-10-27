cmake_minimum_required(VERSION 3.16)
project(TheUninvited CXX C)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(GAMECORE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/engine/gamecorev0.1.1/gamecorev0.1.1")
if(EXISTS "${GAMECORE_DIR}")
  add_subdirectory("${GAMECORE_DIR}" engine EXCLUDE_FROM_ALL)
else()
  message(FATAL_ERROR "Directory ${GAMECORE_DIR} does not exist. Please verify the path.")
endif()
set(DOCTEST_DIR "${GAMECORE_DIR}/inventory/third_party/doctest")

add_library(uninvited_character STATIC src/character/Profession.cpp src/character/CharacterSheet.cpp src/character/CharacterFile.cpp src/character/CharacterProfile.cpp)
target_include_directories(uninvited_character PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/character")
target_link_libraries(uninvited_character PUBLIC gamecore_engine)

add_library(uninvited_menu STATIC src/menu/CharacterStore.cpp src/menu/MenuFlow.cpp src/menu/PlayerStatsCommand.cpp src/menu/RemoteStatsCommand.cpp src/menu/Outfits.cpp src/menu/Preferences.cpp)
target_include_directories(uninvited_menu PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/menu")
target_link_libraries(uninvited_menu PUBLIC uninvited_character)

add_executable(game src/main.cpp src/menu/MenuScreens.cpp src/menu/Focus.cpp src/game/Gamepad.cpp src/game/InputMode.cpp src/menu/MenuSkin.cpp src/menu/MenuScene.cpp src/game/World.cpp src/game/Hud.cpp src/game/Notebook.cpp src/game/Multiplayer.cpp src/game/Sky.cpp src/game/DebugCommands.cpp src/game/Backpack.cpp src/game/InventoryScreen.cpp src/game/BackpackView.cpp src/game/Loot.cpp src/game/LootScreen.cpp src/ui/DpadHud.cpp src/ui/IconArt.cpp src/ui/InteractPrompt.cpp src/ui/ButtonGlyphs.cpp src/interact/InteractionDetector.cpp src/interact/ApproachConfig.cpp src/interact/TargetHud.cpp src/interact/InteractionInput.cpp src/ui/ContextPanel.cpp src/interact/ContainerInteractable.cpp src/interact/TestInteractables.cpp src/interact/ForceBreak.cpp)
target_include_directories(game PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/game" "${CMAKE_CURRENT_SOURCE_DIR}/src/ui" "${CMAKE_CURRENT_SOURCE_DIR}/src/interact")
target_link_libraries(game PRIVATE uninvited_menu gamecore_engine game_net_session gamecore_inventory)

enable_testing()
add_executable(character_tests tests/character/main.cpp tests/character/profession_test.cpp tests/character/attributes_test.cpp
  tests/character/skills_test.cpp tests/character/survival_test.cpp tests/character/sheet_test.cpp tests/character/file_test.cpp tests/character/profile_test.cpp)
target_include_directories(character_tests PRIVATE "${DOCTEST_DIR}")
target_compile_definitions(character_tests PRIVATE CHARACTER_CONFIG_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/config")
target_link_libraries(character_tests PRIVATE uninvited_character)
add_test(NAME character_tests COMMAND character_tests)

add_executable(menu_tests tests/menu/main.cpp tests/menu/preferences_test.cpp tests/menu/store_test.cpp tests/menu/flow_test.cpp tests/menu/appearance_test.cpp)
target_include_directories(menu_tests PRIVATE "${DOCTEST_DIR}")
target_compile_definitions(menu_tests PRIVATE CHARACTER_CONFIG_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/config"
  MENU_TEST_DIR="${CMAKE_CURRENT_BINARY_DIR}/menu_tests_data")
target_link_libraries(menu_tests PRIVATE uninvited_menu game_net_session)
add_test(NAME menu_tests COMMAND menu_tests)

add_executable(ui_tests tests/menu/main.cpp tests/menu/focus_test.cpp src/menu/Focus.cpp tests/game/gamepad_test.cpp tests/game/input_mode_test.cpp src/game/Gamepad.cpp src/game/InputMode.cpp tests/menu/ui_test.cpp tests/menu/scene_test.cpp tests/menu/screens_test.cpp
  tests/game/world_test.cpp tests/game/hud_test.cpp tests/game/notebook_test.cpp tests/game/multiplayer_test.cpp tests/game/sky_test.cpp tests/game/commands_test.cpp tests/game/backpack_test.cpp tests/game/inventory_screen_test.cpp tests/game/loot_test.cpp tests/game/loot_screen_test.cpp tests/ui/dpad_hud_test.cpp tests/interact/interaction_detector_test.cpp tests/interact/approach_test.cpp tests/interact/interaction_input_test.cpp tests/ui/context_panel_test.cpp tests/interact/execution_test.cpp tests/ui/icon_art_test.cpp tests/ui/interact_prompt_test.cpp tests/ui/button_glyphs_test.cpp src/menu/MenuSkin.cpp src/menu/MenuScene.cpp src/menu/MenuScreens.cpp src/game/World.cpp
  src/game/Hud.cpp src/game/Notebook.cpp src/game/Multiplayer.cpp src/game/Sky.cpp src/game/DebugCommands.cpp src/game/Backpack.cpp src/game/InventoryScreen.cpp src/game/BackpackView.cpp src/game/Loot.cpp src/game/LootScreen.cpp src/ui/DpadHud.cpp src/ui/IconArt.cpp src/ui/InteractPrompt.cpp src/ui/ButtonGlyphs.cpp src/interact/InteractionDetector.cpp src/interact/ApproachConfig.cpp src/interact/TargetHud.cpp src/interact/InteractionInput.cpp src/ui/ContextPanel.cpp src/interact/ContainerInteractable.cpp src/interact/TestInteractables.cpp src/interact/ForceBreak.cpp engine/gamecorev0.1.1/gamecorev0.1.1/engine/RecordingCanvas.cpp)
target_include_directories(ui_tests PRIVATE "${DOCTEST_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/menu" "${CMAKE_CURRENT_SOURCE_DIR}/include/game" "${CMAKE_CURRENT_SOURCE_DIR}/src/ui" "${CMAKE_CURRENT_SOURCE_DIR}/src/interact")
target_compile_definitions(ui_tests PRIVATE ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets")
target_link_libraries(ui_tests PRIVATE uninvited_menu gamecore_engine game_net_session gamecore_inventory)
add_test(NAME ui_tests COMMAND ui_tests WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

foreach(target uninvited_character uninvited_menu game character_tests menu_tests ui_tests)
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

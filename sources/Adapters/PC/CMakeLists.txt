
# PC Adapter Build

set(CMAKE_CXX_STANDARD 17)

find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/platform)

# Add source files
set(PC_SOURCES
    main/main_pc.cpp
    system/picoTrackerSystem.cpp
    timer/PCTimer.cpp
    instruments/PCSamplePool.cpp
)

add_library(platform STATIC 
    platform/platform.cpp
    filesystem/PCFileSystem.cpp
)
target_include_directories(platform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/platform ${CMAKE_CURRENT_SOURCE_DIR}/filesystem)
target_include_directories(platform PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../)
target_link_libraries(platform PRIVATE ${SDL2_LIBRARIES})

add_library(platform_display STATIC 
    display/ili9341.c
    display/chargfx.c
    display/PCGUIWindowImp.cpp
    display/PCGUIFactory.cpp
    display/PCEventManager.cpp
)
target_include_directories(platform_display PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/display)
target_include_directories(platform_display PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../)
target_link_libraries(platform_display PRIVATE ${SDL2_LIBRARIES} platform)

add_library(platform_audio STATIC 
    audio/record.cpp
    audio/PCAudio.cpp
)
target_include_directories(platform_audio PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/platform
    ${CMAKE_CURRENT_SOURCE_DIR}/../../
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Services/Audio
)
target_link_libraries(platform_audio PRIVATE ${SDL2_LIBRARIES} platform)

add_executable(picoTracker ${PC_SOURCES})

target_include_directories(picoTracker PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/platform
    ${CMAKE_CURRENT_SOURCE_DIR}/../../
)

target_link_libraries(picoTracker PRIVATE 
    application
    uiframework_basicdatas
    system_system
    system_timer
    foundation
    services_audio
    ${SDL2_LIBRARIES}
)

# Common definitions
target_compile_definitions(picoTracker PRIVATE
    -DPC_BUILD
    -DUSE_LCD=LCD_SDL
)

cmake_minimum_required(VERSION 3.5)

project(RositaCrossword)

add_executable(${PROJECT_NAME} 
    main.c
    include/graphic.c
    include/player.c
    include/audio.c
    include/common.c
    include/logic.c
    include/words.c
    include/game.c
    include/menu.c
    include/entry.c
)

set(ASSETS_DIR ${CMAKE_SOURCE_DIR}/assets)

add_custom_target(
    copy_assets ALL 
    COMMENT "Copying assets to build directory..."
)

add_custom_command(
    TARGET copy_assets 
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSETS_DIR}/audio" "${CMAKE_BINARY_DIR}/audio"
    COMMENT "Copying audio files..."
)

add_custom_command(
    TARGET copy_assets 
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSETS_DIR}/font" "${CMAKE_BINARY_DIR}/font"
    COMMENT "Copying font files..."
)

add_custom_command(
    TARGET copy_assets 
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSETS_DIR}/img" "${CMAKE_BINARY_DIR}/img"
    COMMENT "Copying img files..."
)

add_dependencies(${PROJECT_NAME} copy_assets)

include(FindPkgConfig)
pkg_search_module(SDL3 REQUIRED sdl3)
if(NOT PSP)
	pkg_search_module(GL REQUIRED gl)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${SDL3_INCLUDE_DIRS}
    ${SDL3_image_INCLUDE_DIRS}
    ${SDL3_ttf_INCLUDE_DIRS}
    $ENV{PSPDEV}/psp/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${SDL3_LIBRARIES}
    ${SDL3_image_LIBRARIES}
    ${SDL3_ttf_LIBRARIES}

    SDL3 
    SDL3_image
    SDL3_ttf

    pspaudio
    pspaudiolib

    # dependencias do SDL_mixer
    vorbis # aqui é dependencia vorbis
    ogg # aqui é arquivos .ogg
    modplug # para formatos de músicas tracker

    # DEPENDÊNCIAS DO SDL_TTF
    harfbuzz
    freetype

    png
    jpeg
    z
    bz2 # dependencia de SDL_TFF - bz2 eh a abreviação do linker para BZIP2

    pspctrl 
    pspgum 
    pspgu 
    pspdisplay 
    pspdebug
    pspge
    pspvram
    #pspgles # - PSPDEV/LIB não tem libpspgles.a
    m
)


if(NOT PSP)
	target_include_directories(${PROJECT_NAME} PRIVATE ${GL_INCLUDE_DIRS})
	target_link_libraries(${PROJECT_NAME} PRIVATE ${GL_LIBRARIES})
endif()

if(PSP)
	create_pbp_file(
		TARGET ${PROJECT_NAME}
		ICON_PATH "img/ICON0.png"
		BACKGROUND_PATH "img/PIC1.png"
		PREVIEW_PATH NULL
		TITLE "${PROJECT_NAME}"
	)
endif()
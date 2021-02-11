find_package(Qt5 COMPONENTS LinguistTools)

if (NOT Qt5LinguistTools_FOUND)
    message(WARNING "Qt5LinguistTools - translations won't be either updated or compiled.")
else ()
    # Create translations for everything. Add here every supported locale we have.
    set(DevMode_TS
        de.ts
        es.ts
        it.ts)

    if (REFRESH_TRANSLATIONS)
        file(GLOB_RECURSE translate_SRCS ${CMAKE_SOURCE_DIR}/*.h ${CMAKE_SOURCE_DIR}/*.cpp)
        file(GLOB_RECURSE translate_UIS ${CMAKE_SOURCE_DIR}/ui/*.ui)
        set(translate_SRCS ${translate_SRCS} ${translate_UIS})

        qt5_create_translation(DevMode_QMS ${translate_SRCS} ${DevMode_TS})
    else ()
        qt5_add_translation(DevMode_QMS ${translate_SRCS} ${DevMode_TS})
    endif ()

    add_custom_target(translations ALL DEPENDS ${DevMode_QMS} ${DevMode_TS})

    if (REFRESH_TRANSLATIONS)
        add_custom_target(translations_src DEPENDS ${DevMode_QMS_SRC})
    endif (REFRESH_TRANSLATIONS)
endif()

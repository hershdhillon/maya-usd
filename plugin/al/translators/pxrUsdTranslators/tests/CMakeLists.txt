add_test(
    NAME TestPxrUsdTranslators
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND
        testPxrTranslators.sh
        ${AL_INSTALL_PREFIX}
        ${USD_LIBRARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_INSTALL_PREFIX}/plugin/pxr
    )

DEPENDENT_OPTION( BUILD_DEMOTOOL_TEST "Build Demotool Sensor Components Test" OFF "BUILD_HARDWARE;BUILD_TESTS;BUILD_DEMOTOOL" OFF)

IF ( BUILD_DEMOTOOL_TEST )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )

    # Add a test program
    GLOBAL_ADD_TEST( demotool ${SRCS} )
    # Use  TARGET_LINK_LIBRARIES( report libs... ) to add library deps.
    PROGRAM_ADD_DEPS( demotool orocos-taskbrowser orocos-reporting
      orocos-demotool orocos-kdl orocos-kdltk)

ENDIF ( BUILD_DEMOTOOL_TEST )

DEPENDENT_OPTION( BUILD_IO "Build IO Component" ON "BUILD_HARDWARE" OFF)

#Always process subdirs
#ADD_SUBDIRECTORY( tests )

IF ( BUILD_IO )

    # This gathers all the .cpp files into the variable 'SRCS'
    FILE( GLOB SRCS [^.]*.cpp )
    FILE( GLOB HPPS [^.]*.hpp )
    FILE( GLOB DEVSRCS dev/[^.]*.cpp )
    FILE( GLOB DEVHPPS dev/[^.]*.hpp )

    GLOBAL_ADD_COMPONENT( orocos-io ${SRCS} ${DEVSRCS})

    GLOBAL_ADD_INCLUDE( ocl ${HPPS} )
    GLOBAL_ADD_INCLUDE( ocl/dev ${DEVHPPS} )

ENDIF ( BUILD_IO )

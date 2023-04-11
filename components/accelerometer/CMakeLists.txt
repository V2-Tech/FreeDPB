file(GLOB SRCS
    devices/*.cpp
    *.cpp
    )

file(GLOB INCL
    devices/include
    include
    )

idf_component_register(SRCS ${SRCS}
                       INCLUDE_DIRS ${INCL}
                       PRIV_REQUIRES driver freertos)
file(GLOB SRCS
    dshot/*.c
    *.cpp
    )

file(GLOB INCL
    dshot/include
    include
    )

idf_component_register(SRCS ${SRCS}
                       INCLUDE_DIRS ${INCL}
                       PRIV_REQUIRES driver freertos)
file(GLOB SRCS
    *.cpp
    )

file(GLOB INCL
    include
    )

idf_component_register(SRCS ${SRCS}
                       INCLUDE_DIRS ${INCL}
                       PRIV_REQUIRES driver freertos)
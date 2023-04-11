file(GLOB SRCS
    GUI/*.cpp
    GUI/fonts/*.c
    GUI/images/*.c
    FreeDPB/*.cpp
    sanity/*.cpp
    *.cpp
    )

file(GLOB INCL
    GUI
    FreeDPB
    sanity
    include
    )

idf_component_register(SRCS ${SRCS}
                       INCLUDE_DIRS ${INCL}
                       REQUIRES driver freertos esp-dsp nvs_flash accelerometer rpm-tracker dpb-dsp motor-dshot)
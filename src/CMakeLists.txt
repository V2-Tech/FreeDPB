file(GLOB SRCS
    tasks/*.cpp
    GUI/*.cpp
    GUI/images/*.c
    GUI/fonts/*.c
    sensors/*.cpp
    sensors/drivers/*.cpp
    motor/*.cpp
    motor/dshot/*.c
    app/*.cpp
    shared/*.cpp
    common/*.cpp
    main.cpp
    )

file(GLOB INCL
    tasks
    GUI
    GUI/icons
    GUI/images
    GUI/pages
    sensors
    sensors/drivers
    motor
    motor/dshot
    app
    shared
    sanity
    common
    )

idf_component_register(SRCS ${SRCS}
                       INCLUDE_DIRS ${INCL}
                       REQUIRES driver freertos esp-dsp)
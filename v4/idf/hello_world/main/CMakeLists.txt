file(GLOB SRCS
     *.c
     *.cpp
     i2s_parallel_driver/*.c
     rmt/*.c
     )
idf_component_register(SRCS ${SRCS}
                    INCLUDE_DIRS ".")

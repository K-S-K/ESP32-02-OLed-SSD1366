idf_component_register(SRCS "blink.c" "ssd1366_drv.c" 
                    INCLUDE_DIRS ".")

target_compile_options(${COMPONENT_LIB} PRIVATE -std=gnu++2b)
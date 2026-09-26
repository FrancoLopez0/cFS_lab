project(CFS_ACS C)

add_cfe_app(acs fsw/src/acs_app.c)

target_sources(acs PRIVATE
    libs/mpu6050/mpu6050.c
)

target_include_directories(acs PRIVATE
    libs/mpu6050
    fsw/src
)

target_link_libraries(
  acs
  m
)

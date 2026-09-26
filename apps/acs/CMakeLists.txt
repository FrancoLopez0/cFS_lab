project(CFE_SAMPLE_APP C)

set(APP_SRC_FILES
  fsw/src/acs_app.c
)

# Create the app module
add_cfe_app(acs_app ${APP_SRC_FILES})

target_sources(acs PRIVATE
    libs/mpu6050/mpu6050.c
)

target_include_directories(acs PRIVATE
    libs/mpu6050
)

target_link_libraries(
  acs
  m
)

target_include_directories(acs_app PUBLIC fsw/inc)
# Include the public API from sample_lib to demonstrate how
# to call library-provided functions
add_cfe_app_dependency(acs_app sample_lib)

# Add table
add_cfe_tables(acs_app fsw/tables/acs_app_tbl.c)


project(HELLO_WORLD_APP C)

set(APP_SRC_FILES
  fsw/src/hello_world_app.c
)

# Create the app module
add_cfe_app(hello_world_app ${APP_SRC_FILES})

# target_include_directories(hello_world_app PUBLIC fsw/inc)

# Include the public API from sample_lib to demonstrate how
# to call library-provided functions

# add_cfe_app_dependency(hello_world_app sample_lib)

# Add table
# add_cfe_tables(hello_world_app fsw/tables/hello_world_app_tbls.c)

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
# if (ENABLE_UNIT_TESTS)
#   add_subdirectory(unit-test)
# endif (ENABLE_UNIT_TESTS)

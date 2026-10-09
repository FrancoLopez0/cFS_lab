project(CFS_MCS C)

add_cfe_app(mcs fsw/src/mcs_app.c)

target_sources(mcs PRIVATE
    libs/pwm/pwm.c
    fsw/src/mcs_dispatch.c
    fsw/src/mcs_cmds.c
)

target_include_directories(mcs PRIVATE
    libs/pwm
    fsw/src
)

target_include_directories(mcs PUBLIC
    fsw/inc
)
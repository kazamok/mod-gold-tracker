set(MODULE_NAME "mod-gold-tracker")
set(MODULE_SRCS
    src/mod-gold-tracker.cpp
)

# Add module to the build
add_azerothcore_module(${MODULE_NAME} ${MODULE_SRCS})

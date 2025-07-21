set(MODULE_NAME "mod-gold-tracker")
set(MODULE_SRCS
    src/mod-gold-tracker.cpp
)

AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/mod-gold-tracker.cpp")

# 설정 파일 설치
install(FILES
    conf/mod-gold-tracker.conf.dist
    DESTINATION "${CMAKE_INSTALL_PREFIX}/conf"
)
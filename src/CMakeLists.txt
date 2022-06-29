aux_source_directory(. SRC_LIST)

add_executable(RCS ${SRC_LIST})

target_link_libraries(RCS event mysqlclient pthread)
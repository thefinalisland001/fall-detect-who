set(src_dirs        ./)

set(include_dirs    ./)

set(requires who_detect_app)

idf_component_register(SRC_DIRS ${src_dirs} INCLUDE_DIRS ${include_dirs} REQUIRES ${requires})
idf_component_optional_requires(PUBLIC fall_detect espressif__fall_detect)

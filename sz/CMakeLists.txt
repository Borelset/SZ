aux_source_directory(src sz_sources)

add_library (SZ SHARED ${sz_sources})
message("${sz_sources}")
target_link_libraries (SZ zlib zstd ${OpenCL_LIBRARIES})
target_compile_options(SZ
	PRIVATE $<$<CONFIG:Debug>:-Wall -Wextra -Wpedantic -Wno-unused-parameter>
	)

install (TARGETS SZ LIBRARY DESTINATION lib)


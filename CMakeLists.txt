add_library(ps2intrin INTERFACE)

set_target_properties(ps2intrin PROPERTIES LINKER_LANGUAGE CXX)

target_sources(ps2intrin
	PUBLIC
	"include/ps2intrin.h"
)

target_include_directories(ps2intrin SYSTEM INTERFACE "intrin/")

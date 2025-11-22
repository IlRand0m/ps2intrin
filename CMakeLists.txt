add_library(ps2intrin INTERFACE)

target_sources(ps2intrin
	PUBLIC
	"include/ps2intrin.h"
)

target_include_directories(ps2intrin SYSTEM INTERFACE "include/")

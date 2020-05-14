#include <filesystem>


#ifndef global_config
#define global_config

// Temp dir specifically for aroww db test suit, removed at the end
#define TEMP_DIR (std::filesystem::temp_directory_path() / "aroww-db-tests")


#endif

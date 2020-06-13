#pragma once

#define SPDLOG_FMT_EXTERNAL 1
#include "spdlog/spdlog.h"

#define log_debug(...) (spdlog::info(__VA_ARGS__))
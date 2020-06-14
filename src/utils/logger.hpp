#pragma once

#define SPDLOG_FMT_EXTERNAL 1
#include "spdlog/spdlog.h"

#define log_debug(...) (spdlog::debug(__VA_ARGS__))
#define log_info(...) (spdlog::info(__VA_ARGS__))

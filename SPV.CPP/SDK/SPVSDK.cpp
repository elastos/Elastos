#include <string>
#include <iostream>
#include "SPVSDK.h"
#include "spdlog/spdlog.h"
#include "nlohmann/json.hpp"

std::string SPVSDK::version()
{
	// Console logger with color
#if 0
	auto console = spdlog::stdout_color_mt("console");
	console->info("Welcome to spdlog!");
	console->error("Some error message with arg{}..", 1);
#endif

#if 0
	nlohmann::json j;
	j.push_back({"hello", "world"});
#endif

	return "v1.0.0";
}

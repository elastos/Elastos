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
	std::string arg = "hello";
	int argc = 1;
	const char *argv[1] = {arg.c_str()};
	Catch::Session session; // There must be exactly one instance

	// writing to session.configData() here sets defaults
	// this is the preferred way to set them

	int returnCode = session.applyCommandLine( argc, argv );
	if( returnCode != 0 ) // Indicates a command line error
	  return std::string("" + returnCode);

	// writing to session.configData() or session.Config() here
	// overrides command line args
	// only do this if you know you need to

	int numFailed = session.run();

	// numFailed is clamped to 255 as some unices only use the lower 8 bits.
	// This clamping has already been applied, so just return it here
	// You can also do any post run clean-up here
#endif

#if 0
	nlohmann::json j;
	j.push_back({"hello", "world"});
#endif

	return "v1.0.0";
}

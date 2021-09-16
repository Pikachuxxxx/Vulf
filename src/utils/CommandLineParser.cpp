#include "CommandLineParser.h"

#include <iomanip>
#include <assert.h>

CommandLineParser::CommandLineParser()
{
    Add("help", { "--help" }, 0, "Show help");
	Add("validation", {"-v", "--validation"}, 0, "Enable validation layers");
	Add("width", { "-w", "--width" }, 1,   "Set window width");
	Add("height", { "-h", "--height" }, 1, "Set window height");
    // Add("vsync", {"-vs", "--vsync"}, 0, "Enable V-Sync");
    // Add("fullscreen", { "-f", "--fullscreen" }, 0, "Start in fullscreen mode");
    // Add("gpulist", { "-gl", "--listgpus" }, 0, "Display a list of available Vulkan devices");
	// Add("gpuselection", { "-g", "--gpu" }, 1, "Select GPU to run on");
	// Add("benchmark", { "-b", "--benchmark" }, 0, "Run example in benchmark mode");
	// Add("benchmarkwarmup", { "-bw", "--benchwarmup" }, 1, "Set warmup time for benchmark mode in seconds");
	// Add("benchmarkruntime", { "-br", "--benchruntime" }, 1, "Set duration time for benchmark mode in seconds");
	// Add("benchmarkresultfile", { "-bf", "--benchfilename" }, 1, "Set file name for benchmark results");
	// Add("benchmarkresultframes", { "-bt", "--benchframetimes" }, 0, "Save frame times to benchmark results file");
	// Add("benchmarkframes", { "-bfs", "--benchmarkframes" }, 1, "Only render the given number of frames");
}

void CommandLineParser::Add(std::string name, std::vector<std::string> commands, bool hasValue, std::string help)
{
    options[name].commands = commands;
    options[name].value = "";
    options[name].hasValue = hasValue;
	options[name].help = help;
	options[name].set = false;
}

void CommandLineParser::PrintHelp()
{
    std::cout << "Available command line options:\n";
	for (auto option : options) {
		std::cout << " ";
		for (size_t i = 0; i < option.second.commands.size(); i++) {
			std::cout << option.second.commands[i];
			if (i < option.second.commands.size() - 1) {
				std::cout << ", ";
			}
		}
		std::cout << ": " << option.second.help << "\n";
	}
}

void CommandLineParser::Parse(std::vector<const char*> arguments)
{
    bool printHelp = false;
	// Known arguments
	for (auto& option : options) {
		for (auto& command : option.second.commands) {
			for (size_t i = 0; i < arguments.size(); i++) {
				if (strcmp(arguments[i], command.c_str()) == 0) {
					option.second.set = true;
					// Get value
					if (option.second.hasValue) {
						if (arguments.size() > i + 1) {
							option.second.value = arguments[i + 1];
						}
						if (option.second.value == "") {
							printHelp = true;
							break;
						}
					}
				}
			}
		}
	}
	// Print help for unknown arguments or missing argument values
	if (printHelp) {
		options["help"].set = true;
	}
}

bool CommandLineParser::IsSet(std::string name)
{
    return ((options.find(name) != options.end()) && options[name].set);
}

std::string CommandLineParser::GetValueAsString(std::string name, std::string defaultValue)
{
    assert(options.find(name) != options.end());
	std::string value = options[name].value;
	return (value != "") ? value : defaultValue;
}

int32_t CommandLineParser::GetValueAsInt(std::string name, int32_t defaultValue)
{
    assert(options.find(name) != options.end());
	std::string value = options[name].value;
	if (value != "") {
		char* numConvPtr;
		int32_t intVal = strtol(value.c_str(), &numConvPtr, 10);
		return (intVal > 0) ? intVal : defaultValue;
	} else {
		return defaultValue;
	}
	return int32_t();
}

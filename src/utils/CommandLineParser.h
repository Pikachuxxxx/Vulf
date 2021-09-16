#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

class CommandLineParser
{
	struct CommandLineOption
	{
		std::vector<std::string> commands;
		std::string value;
		bool hasValue = false;
		std::string help;
		bool set = false;
	};
public:
	CommandLineParser();
	void Add(std::string name, std::vector<std::string> commands, bool hasValue, std::string help);
	void PrintHelp();
	void Parse(std::vector<const char*> arguments);
	bool IsSet(std::string name);
	std::string GetValueAsString(std::string name, std::string defaultValue);
	int32_t GetValueAsInt(std::string name, int32_t defaultValue);
public:
	std::unordered_map<std::string, CommandLineOption> options;
};

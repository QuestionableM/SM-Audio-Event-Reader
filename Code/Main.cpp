#include "EventReader.h"
#include <iostream>

#include <filesystem>
namespace fs = std::filesystem;

bool exists_safe(const std::string& path)
{
	std::error_code ec;
	bool exists = fs::exists(path, ec);

	return (exists && !ec);
}

int main()
{
	SMEventReader event_reader;
	if (!event_reader.Initialize())
		return -1;

program_init:
	std::string bank_file, bank_strings_file;

	std::cout << "Path to Bank File: ";
	std::getline(std::cin, bank_file);

	std::cout << "Path to Bank Strings File: ";
	std::getline(std::cin, bank_strings_file);

	if (!exists_safe(bank_file))
	{
		std::cerr << "Path to bank file is invalid!\n";
		goto program_init;
	}

	if (!exists_safe(bank_strings_file))
	{
		std::cerr << "Path to bank strings file is invalid!\n";
		goto program_init;
	}

	event_reader.ReadEvents(bank_file, bank_strings_file);
	event_reader.WriteEvents();

	return 0;
}
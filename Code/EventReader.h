#pragma once

#include <string>
#include <vector>

#include <fmod\fmod_studio.hpp>

struct FMODEventParameterData
{
	std::string name;
	float min_value;
	float max_value;
	float default_value;
};

struct FMODEventData
{
	std::string name;
	std::vector<FMODEventParameterData> parameters;
};

class SMEventReader
{
	std::vector<FMODEventData> m_CachedEvents;

	FMOD::Studio::System* m_System;
	FMOD::Studio::Bank* m_MasterBank;
	FMOD::Studio::Bank* m_MasterBankStrings;

public:
	SMEventReader() = default;
	SMEventReader(const SMEventReader&&) = delete;
	SMEventReader(const SMEventReader&) = delete;
	SMEventReader(SMEventReader&&) = delete;
	~SMEventReader();

	bool Initialize();

	void ReadEvents(const std::string& bank_path, const std::string& bank_strings_path);
	void WriteEvents();
};
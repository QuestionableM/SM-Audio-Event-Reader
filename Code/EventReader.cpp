#include "EventReader.h"

#include <json\json.hpp>

#include <iostream>
#include <iomanip>
#include <fstream>

SMEventReader::~SMEventReader()
{

}

bool SMEventReader::Initialize()
{
	FMOD_RESULT fr = FMOD::Studio::System::create(&m_System);
	if (fr != FMOD_OK)
	{
		std::cerr << "Couldn't create FMOD Studio System\n";
		return false;
	}

	fr = m_System->initialize(32, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr);
	if (fr != FMOD_OK)
	{
		std::cerr << "Couldn't initialize FMOD Studio System\n";
		return false;
	}

	std::cout << "FMOD Studio System has been successfully initialized!\n";
	return true;
}

void SMEventReader::ReadEvents(const std::string& bank_path, const std::string& bank_strings_path)
{
	m_CachedEvents.clear();

	FMOD_RESULT fr = m_System->loadBankFile(bank_path.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &m_MasterBank);
	if (fr != FMOD_OK)
	{
		std::cerr << "Load Bank File: Invalid File\n";
		return;
	}

	fr = m_System->loadBankFile(bank_strings_path.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &m_MasterBankStrings);
	if (fr != FMOD_OK)
	{
		std::cerr << "Load Bank Strings File: Invalid File\n";
		return;
	}

	std::cout << "Successfully loaded the bank files!\n";

	int event_count;
	fr = m_MasterBank->getEventCount(&event_count);
	if (fr != FMOD_OK)
	{
		std::cerr << "Couldn't get the amount of events in a bank file\n";
		return;
	}

	std::cout << "Found " << event_count << " events\n";

	std::vector<FMOD::Studio::EventDescription*> m_DescriptionStorage;
	m_DescriptionStorage.resize(static_cast<std::size_t>(event_count));

	fr = m_MasterBank->getEventList(m_DescriptionStorage.data(), event_count, &event_count);
	if (fr != FMOD_OK)
	{
		std::cerr << "Couldn't get the event list\n";
		return;
	}

	for (int a = 0; a < event_count; a++)
	{
		FMOD::Studio::EventDescription* cur_event = m_DescriptionStorage[a];

		std::string event_path;
		event_path.resize(200);
		int event_path_sz;
		if (cur_event->getPath(event_path.data(), static_cast<int>(event_path.size()), &event_path_sz) != FMOD_OK)
			continue;

		FMODEventData new_event;
		new_event.name = event_path.substr(0, event_path_sz - 1);

		std::cout << a << " -> " << new_event.name << ":\n";

		int desc_count;
		if (cur_event->getParameterDescriptionCount(&desc_count) != FMOD_OK)
			continue;

		for (int b = 0; b < desc_count; b++)
		{
			FMOD_STUDIO_PARAMETER_DESCRIPTION param_desc;
			if (cur_event->getParameterDescriptionByIndex(b, &param_desc) != FMOD_OK)
			{
				std::cerr << "Couldn't get parameter " << b << " by index\n";
				continue;
			}

			FMODEventParameterData new_parameter;
			new_parameter.name = std::string(param_desc.name);
			new_parameter.default_value = param_desc.defaultvalue;
			new_parameter.min_value = param_desc.minimum;
			new_parameter.max_value = param_desc.maximum;

			std::cout << "\t" << a << " -> { Name: " << new_parameter.name << ", Min: " << param_desc.minimum << ", Max: " << param_desc.maximum << ", Default: " << param_desc.defaultvalue << " }\n";

			new_event.parameters.push_back(new_parameter);
		}

		m_CachedEvents.push_back(new_event);
	}

	std::cout << "Successfully loaded " << m_CachedEvents.size() << " into memory!\n";
}

void SMEventReader::WriteEvents()
{
	std::ofstream out_file("./sm_fmod_events.json");
	if (!out_file.is_open())
	{
		std::cerr << "Couldn't write the event data to file\n";
		return;
	}

	nlohmann::json output_data = nlohmann::json::object();

	std::cout << "Writing " << m_CachedEvents.size() << " events to file\n";
	for (std::size_t a = 0; a < m_CachedEvents.size(); a++)
	{
		const FMODEventData& cur_data = m_CachedEvents[a];

		nlohmann::json cur_data_obj = nlohmann::json::object();
		if (!cur_data.parameters.empty())
		{
			nlohmann::json params_json_obj = nlohmann::json::object();

			for (std::size_t b = 0; b < cur_data.parameters.size(); b++)
			{
				const FMODEventParameterData& cur_param_data = cur_data.parameters[b];

				nlohmann::json cur_param_json = nlohmann::json::object();
				cur_param_json["minimum"] = cur_param_data.min_value;
				cur_param_json["maximum"] = cur_param_data.max_value;
				cur_param_json["default"] = cur_param_data.default_value;

				params_json_obj[cur_param_data.name] = cur_param_json;
			}

			cur_data_obj["Parameters"] = params_json_obj;
		}

		output_data[cur_data.name] = cur_data_obj;
	}

	out_file << std::setw(1) << std::setfill('\t') << output_data;
	out_file.close();
}
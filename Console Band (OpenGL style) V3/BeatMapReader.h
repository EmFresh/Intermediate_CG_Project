#pragma once
#include <vector>
#include <fstream>
#include <string>
//#include <minibpm/MiniBpm.h>
#include "Utilities.h"
#include "EmGineAudioPlayer.h"

struct Beat { uint timeMS = 0; float bpm = 0; };

class BeatMapReader
{
public:
	static std::vector<Beat> loadBeatMap(cstring file)
	{
		std::vector<Beat> beater;

		std::ifstream ifs(file);

		std::string line;
		while(std::getline(ifs, line))
		{
			line = tolower((char*)line.c_str());

			if(strstr(line.c_str(), "#"))continue;
			if(strstr(line.c_str(), "bpm_list"))
			{//this assumes the bpm stays the same throughout the track
				while(true)
				{
					uint begin = line.find('{'), end = line.find('}');
					if(begin == std::string::npos || end == std::string::npos)break;

					std::string tmp = line.substr(end + 1);
					line = line.substr(begin + 1, end - begin - 1);

					beater.push_back({(uint)std::stoi(line.substr(0, line.find(':'))),(float)std::stoi(line.substr(line.find(':') + 1))});
					line = tmp;
				}
			}
			//if(strstr(line.c_str(),"sub")){}
		}

		return beater;
	}

	static Beat calculateBPM(float* data, uint sampleSize, float sampleRate)
	{
		//	breakfastquay::MiniBPM bpm(sampleRate);
		//	auto bpmf =bpm.estimateTempoOfSamples(data, sampleSize);
		return {0,0};
	}
};


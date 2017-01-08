#pragma once
#include <string>
#include "common.h"
using namespace std;

struct Parameters
{
	float MyPi;
	string InputFilePath;
	float MyEpsilon;
	int MyInfinity;
	int ResolutionMultipe;
	float ThresholdForSeparateLines;
};


extern Config::Ptr g_config;
extern Parameters g_parameters;

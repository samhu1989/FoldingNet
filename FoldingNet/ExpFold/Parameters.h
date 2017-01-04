#pragma once
#include <string>

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



extern Parameters g_parameters;

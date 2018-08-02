#pragma once

#include "cinder/Vector.h"
#include "Configurable.h"

// this class is an example how to use Configurable
class AClass : public Configurable
{
public:
	void readConfig(Bit::JsonTree* tree);
	void readParams(Bit::JsonTree* tree, Bit::ParamsRef params);

private:
	// these fields would be read from config.json
	bool configBool_;
	double configDouble_;
	float configFloat_;
	std::string configString_;
	int configInt_;
	ci::Vec2i configVec2i_;
	ci::Vec2f configVec2f_;
	ci::Vec3f configVec3f_;

	// these fields would be read from params.json and added into params
	bool paramsBool_;
	double paramsDouble_;
	float paramsFloat_;
	std::string paramsString_;
	int paramsInt_;
	ci::Vec2i paramsVec2i_;
	ci::Vec2f paramsVec2f_;
	ci::Vec3f paramsVec3f_;
};

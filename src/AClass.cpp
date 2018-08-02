#include "AClass.h"

void AClass::readConfig(Bit::JsonTree* tree)
{
	configBool_ = tree->getChildPtr("configBool")->getValueT<bool>();
	configDouble_ = tree->getChildPtr("configDouble")->getValueT<double>();
	configFloat_ = tree->getChildPtr("configFloat")->getValueT<float>();
	configString_ = tree->getChildPtr("configString")->getValueT<std::string>();
	configInt_ = tree->getChildPtr("configInt")->getValueT<int>();
	configVec2i_ = tree->getChildPtr("configVec2i")->getValueT<ci::Vec2i>();
	configVec2f_ = tree->getChildPtr("configVec2f")->getValueT<ci::Vec2f>();
	configVec3f_ = tree->getChildPtr("configVec3f")->getValueT<ci::Vec3f>();
}

void AClass::readParams(Bit::JsonTree* tree, Bit::ParamsRef params)
{
	params->addParam<bool>(tree->getChildPtr("paramsBool"), paramsBool_);
	params->addParam<double>(tree->getChildPtr("paramsDouble"), paramsDouble_);
	params->addParam<float>(tree->getChildPtr("paramsFloat"), paramsFloat_);
	params->addParam<std::string>(tree->getChildPtr("paramsString"), paramsString_);
	params->addParam<int>(tree->getChildPtr("paramsInt"), paramsInt_);
	params->addParam<ci::Vec2i>(tree->getChildPtr("paramsVec2i"), paramsVec2i_);
	params->addParam<ci::Vec2f>(tree->getChildPtr("paramsVec2f"), paramsVec2f_);
	params->addParam<ci::Vec3f>(tree->getChildPtr("paramsVec3f"), paramsVec3f_);
}

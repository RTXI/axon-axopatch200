#include <QtGui>
#include "axon-axopatch-200.h"
#include <iostream>

extern "C" Plugin::Object * createRTXIPlugin(void) {
	return new AxoPatch();
};

static DefaultGUIModel::variable_t vars[] = {
	{ "Mode Input", "", DefaultGUIModel::INPUT, },
	{ "Mode Output", "", DefaultGUIModel::OUTPUT, },
	{ "Input Channel", "", DefaultGUIModel::PARAMETER | DefaultGUIModel::UINTEGER, },
	{ "Output Channel", "", DefaultGUIModel::PARAMETER | DefaultGUIModel::UINTEGER, },
	{ "VClamp Gain", "", DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, }, 
	{ "IClamp Gain", "", DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
};

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);

AxoPatch::AxoPatch(void) : DefaultGUIModel("Axon AxoPatch 700 Controller", ::vars, ::num_vars) {
	setWhatsThis("<p>Yeah, I'll get to this later... <br>-Ansel</p>");
	DefaultGUIModel::createGUI(vars, num_vars);
	initParameters();
//	customizeGUI();
	update( INIT );
	refresh();
};

AxoPatch::~AxoPatch(void) {};

void AxoPatch::initParameters(void) {
	input_channel = 0;
	output_channel = 1;

	vclamp_gain = iclamp_gain = 1;
};

void AxoPatch::update(DefaultGUIModel::update_flags_t flag) {
	switch(flag) {
		case INIT:
			setParameter("Input Channel", input_channel);
			setParameter("Output Channel", output_channel);
			setParameter("VClamp Gain", vclamp_gain);
			setParameter("IClamp Gain", iclamp_gain);
			break;
		
		case MODIFY:
			break;

		case PAUSE:
			break;

		case UNPAUSE:
			break;

		case PERIOD:
			break;

		default:
			break;
	}
}

void customizeGUI(void) {
	
}

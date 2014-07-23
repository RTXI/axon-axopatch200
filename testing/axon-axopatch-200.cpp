#include <QtGui>
#include <iostream>
//#include <daq.h>
#include "axon-axopatch-200.h"

extern "C" Plugin::Object * createRTXIPlugin(void) {
	return new AxoPatch();
};

static DefaultGUIModel::variable_t vars[] = {
	{ "Mode Telegraph", "", DefaultGUIModel::INPUT, },
	{ "Gain Telegraph", "", DefaultGUIModel::INPUT, },
	{ "Input Channel", "", DefaultGUIModel::PARAMETER | DefaultGUIModel::INTEGER, },
	{ "Output Channel", "", DefaultGUIModel::PARAMETER | DefaultGUIModel::INTEGER, },
	{ "Headstage Configuration", "", DefaultGUIModel::PARAMETER | DefaultGUIModel::INTEGER, },
	{ "Output Gain", "", DefaultGUIModel::PARAMETER | DefaultGUIModel::INTEGER, }, 
	{ "Amplifier Mode", "", DefaultGUIModel::PARAMETER | DefaultGUIModel::INTEGER, },
};

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);

static void getDevice(DAQ::Device *d, void *p) {
	DAQ::Device **device = reinterpret_cast<DAQ::Device **>(p);

	if (!*device) *device = d;
}

AxoPatch::AxoPatch(void) : DefaultGUIModel("Axon AxoPatch 200 Controller", ::vars, ::num_vars) {
	setWhatsThis("<p>Yeah, I'll get to this later... <br>-Ansel</p>");
	DefaultGUIModel::createGUI(vars, num_vars);
	initParameters();
	customizeGUI();
	update( INIT );
	refresh();
};

AxoPatch::~AxoPatch(void) {};

void AxoPatch::initParameters(void) {
	input_channel = 0;
	output_channel = 1;
	output_ui_index = 0;
	headstage_config = 0;
	amp_mode = 1;
	auto_on = false;
	output_gain = headstage_gain = 0;

	iclamp_ai_gain = 1; // (1 V/V)
	iclamp_ao_gain = 1.0 / 2e-9; // (2 nA/V)
	izero_ai_gain = 1; // (1 V/V)
	izero_ao_gain = 0; // No output
	vclamp_ai_gain = 1e-12/1e-3; // (1 mV/pA)
	vclamp_ao_gain = 1 / 20e-3; // (20 mV/V)
//	iclamp_out_gain;
//	vclamp_out_gain;
};

void AxoPatch::update(DefaultGUIModel::update_flags_t flag) {
	switch(flag) {
		case INIT:
			setParameter("Input Channel", input_channel);
			setParameter("Output Channel", output_channel);
			setParameter("Headstage Configuration", headstage_config);
			setParameter("Output Gain", output_ui_index);
			setParameter("Amplifier Mode", amp_mode);

			inputBox->blockSignals(true);
			inputBox->setValue(input_channel);
			inputBox->blockSignals(false);
			outputBox->blockSignals(true);
			outputBox->setValue(output_channel);
			outputBox->blockSignals(false);
			ampButtonGroup->blockSignals(true);
			ampButtonGroup->button(amp_mode)->setChecked(true);
			ampButtonGroup->blockSignals(false);
			outputGainBox->blockSignals(true);
			outputGainBox->setCurrentIndex(output_ui_index);
			outputGainBox->blockSignals(false);
			headstageBox->blockSignals(true);
			headstageBox->setCurrentIndex(headstage_config);
			headstageBox->blockSignals(false);

			updateDAQ();
			break;
		
		case MODIFY:
			input_channel = getParameter("Input Channel").toInt();
			output_channel = getParameter("Output Channel").toInt();
			output_ui_index = getParameter("Output Gain").toInt();
			headstage_config = getParameter("Headstage Configuration").toInt();
			amp_mode = getParameter("Amplifier Mode").toInt();

			inputBox->setValue(input_channel);
			outputBox->setValue(output_channel);
			ampButtonGroup->button(amp_mode)->setChecked(true);
			outputGainBox->setCurrentIndex(output_ui_index);
			headstageBox->setCurrentIndex(headstage_config);

			updateDAQ();
			break;

		default:
			break;
	}
}

void AxoPatch::execute(void) {
	if (!auto_on) {
		return;
	}

	if (input(0) < 3) temp_mode = 1; //ICLAMP
	else temp_mode = 2; //VCLAMP
	
	if (temp_mode != amp_mode) {
		amp_mode = temp_mode;
		settings_changed = true;
	}
	
	if (input(1) <= .75) temp_gain = .05;
	else if (input(1) <= 1.25) temp_gain = .1;
	else if (input(1) <= 1.75) temp_gain = .2;
	else if (input(1) <= 2.25) temp_gain = .5;
	else if (input(1) <= 2.75) temp_gain = 1.0;
	else if (input(1) <= 3.25) temp_gain = 2.0;
	else if (input(1) <= 3.75) temp_gain = 5.0;
	else if (input(1) <= 4.25) temp_gain = 10;
	else if (input(1) <= 4.75) temp_gain = 20;
	else if (input(1) <= 5.25) temp_gain = 50;
	else if (input(1) <= 5.75) temp_gain = 100;
	else if (input(1) <= 6.25) temp_gain = 200;
	else if (input(1) <= 6.75) temp_gain = 500;

	if (temp_gain != output_gain) {
		output_gain = temp_gain;
		settings_changed = true;
	}
}

void AxoPatch::updateMode(int value) {
	amp_mode = value;
	setParameter("Amplifier Mode", amp_mode);
	
	if (!settings_changed) {
		settings_changed = true;
		updateDAQ();
	}
}

void AxoPatch::updateOutputGain(int value) {
	output_ui_index = value;
	setParameter("Output Gain", output_ui_index);

	switch(value) {
		case 0:
			output_gain = .5;
			break;
		case 1:
			output_gain = 1;
			break;
		case 2:
			output_gain = 2;
			break;
		case 3:
			output_gain = 5;
			break;
		case 4:
			output_gain = 10;
			break;
		case 5:
			output_gain = 20;
			break;
		case 6:
			output_gain = 50;
			break;
		case 7:
			output_gain = 100;
			break;
		case 8:
			output_gain = 200;
			break;
		case 9:
			output_gain = 500;
			break;
		default:
			output_gain = 0;
			break;
	}

	if (!settings_changed) {
		settings_changed = true;
		updateDAQ();
	}
}

void AxoPatch::updateHeadstageBox(int value) {
	headstage_config = value;
	setParameter("Headstage Configuration", headstage_config);

	switch(value) {
		case 2: 
			headstage_gain = .1;
			break;

		default:
			headstage_gain = 1;
			break;
	}

	if (!settings_changed) {
		settings_changed = true;
		updateDAQ();
	}
}

void AxoPatch::updateOutputChannel(int value) {
	output_channel = value;
	setParameter("Output Channel", output_channel);

	if (!settings_changed) {
		settings_changed = true;
		updateDAQ();
	}
}

void AxoPatch::updateInputChannel(int value) {
	input_channel = value;
	setParameter("Input Channel", input_channel);
	
	if (!settings_changed) {
		settings_changed = true;
		updateDAQ();
	}
}

void AxoPatch::toggleAutoMode(bool on) {
	if (on) {
		auto_on = true;
		iclampButton->setEnabled(false);
		vclampButton->setEnabled(false);
		outputGainBox->setEnabled(false);
	}
	else {
		auto_on = false;
		iclampButton->setEnabled(true);
		vclampButton->setEnabled(true);
		outputGainBox->setEnabled(true);
		updateDAQ();
	}
	setActive(on);
};

void AxoPatch::updateDAQ(void) {
	if (!device) return;

//	std::cout<<"Nope, didn't return"<<std::endl;
//	return;
	switch(amp_mode) {
		case 1: //IClamp
			device->setAnalogRange(DAQ::AI, input_channel, 0);
			device->setAnalogGain(DAQ::AI, input_channel, iclamp_ai_gain*output_gain*headstage_gain);
			device->setAnalogGain(DAQ::AO, output_channel, iclamp_ao_gain*headstage_gain);
			device->setAnalogCalibration(DAQ::AO, output_channel);
			device->setAnalogCalibration(DAQ::AI, input_channel);
			break;

		case 2: //VClamp
			device->setAnalogRange(DAQ::AI, input_channel, 0);
			device->setAnalogGain(DAQ::AI, input_channel, vclamp_ai_gain*output_gain*headstage_gain);
			device->setAnalogGain(DAQ::AO, output_channel, vclamp_ao_gain);
			device->setAnalogCalibration(DAQ::AO, output_channel);
			device->setAnalogCalibration(DAQ::AI, input_channel);
			break;

		default:
			std::cout<<"ERROR. Something went horribly wrong.\n The amplifier mode is set to an unknown value"<<std::endl;
			break;
	}

	settings_changed = false;
};

void AxoPatch::customizeGUI(void) {
	QGridLayout *customLayout = DefaultGUIModel::getLayout();
	
	customLayout->itemAtPosition(1,0)->widget()->setVisible(false);
	customLayout->itemAtPosition(10,0)->widget()->setVisible(false);

	// create input spinboxes
	QGroupBox *ioBox = new QGroupBox;
	QHBoxLayout *ioBoxLayout = new QHBoxLayout;
	ioBox->setLayout(ioBoxLayout);
	inputBox = new QSpinBox;
	outputBox = new QSpinBox;
	QLabel *inputBoxLabel = new QLabel("Input");
	QLabel *outputBoxLabel = new QLabel("Output");
	ioBoxLayout->addWidget(inputBoxLabel);
	ioBoxLayout->addWidget(inputBox);
	ioBoxLayout->addWidget(outputBoxLabel);
	ioBoxLayout->addWidget(outputBox);

	// create gain and amplifer mode comboboxes
	QFormLayout *comboBoxLayout = new QFormLayout;
	headstageBox = new QComboBox;
	headstageBox->insertItem( 0, trUtf8("\x50\x61\x74\x63\x68\x20\xce\xb2\x3d\x31") );
	headstageBox->insertItem( 1, trUtf8( "\x57\x68\x6f\x6c\x65\x20\x43\x65\x6c\x6c\x20\xce\xb2\x3d\x31" ) );
	headstageBox->insertItem( 2, trUtf8( "\x57\x68\x6f\x6c\x65\x20\x43\x65\x6c\x6c\x20\xce\xb2\x3d\x30\x2e\x31" ) );
	outputGainBox = new QComboBox;
	outputGainBox->insertItem( 0, tr("0.5") );
	outputGainBox->insertItem( 1, tr("1") );
	outputGainBox->insertItem( 2, tr("2") );
	outputGainBox->insertItem( 3, tr("5") );
	outputGainBox->insertItem( 4, tr("10") );
	outputGainBox->insertItem( 5, tr("20") );
	outputGainBox->insertItem( 6, tr("50") );
	outputGainBox->insertItem( 7, tr("100") );
	outputGainBox->insertItem( 8, tr("200") );
	outputGainBox->insertItem( 9, tr("500") );
	comboBoxLayout->addRow("Output Gain", outputGainBox);
	comboBoxLayout->addRow("Headstage", headstageBox);

	// create amp mode groupbox
	QGroupBox *ampModeBox = new QGroupBox;
	QHBoxLayout *ampModeBoxLayout = new QHBoxLayout;
	ampModeBox->setLayout(ampModeBoxLayout);
	ampButtonGroup = new QButtonGroup;
	iclampButton = new QRadioButton("IClamp");
	iclampButton->setCheckable(true);
	ampButtonGroup->addButton(iclampButton, 1);
	vclampButton = new QRadioButton("VClamp");
	vclampButton->setCheckable(true);
	ampButtonGroup->addButton(vclampButton, 2);
	autoButton = new QPushButton("Auto");
	autoButton->setCheckable(true);
	ampModeLabel = new QLabel;
	ampModeLabel->setText("Amp Mode");
	ampModeLabel->setAlignment(Qt::AlignCenter);

	QVBoxLayout *row2Layout = new QVBoxLayout;
	row2Layout->addWidget(autoButton);
	row2Layout->addWidget(ampModeLabel);
	ampModeBoxLayout->addLayout(row2Layout);
	
	QVBoxLayout *row1Layout = new QVBoxLayout;
	row1Layout->setAlignment(Qt::AlignRight);
	row1Layout->addWidget(iclampButton);
	row1Layout->addWidget(vclampButton);
	ampButtonGroup->setExclusive(true);
	ampModeBoxLayout->addLayout(row1Layout);

	// add widgets to custom layout
	customLayout->addWidget(ioBox, 0, 0);
	customLayout->addLayout(comboBoxLayout, 2, 0);
	customLayout->addWidget(ampModeBox, 3, 0);
	setLayout(customLayout);

	QObject::connect(ampButtonGroup, SIGNAL(buttonPressed(int)), this, SLOT(updateMode(int)));
	QObject::connect(outputGainBox, SIGNAL(activated(int)), this, SLOT(updateOutputGain(int)));
	QObject::connect(headstageBox, SIGNAL(activated(int)), this, SLOT(updateHeadstageBox(int)));
	QObject::connect(inputBox, SIGNAL(valueChanged(int)), this, SLOT(updateInputChannel(int)));
	QObject::connect(outputBox, SIGNAL(valueChanged(int)), this, SLOT(updateOutputChannel(int)));
	QObject::connect(autoButton, SIGNAL(toggled(bool)), this, SLOT(toggleAutoMode(bool)));
}

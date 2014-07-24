#include <QtGui>
#include <iostream>
#include "axon-axopatch-200.h"

AxoPatchComboBox::AxoPatchComboBox(QWidget *parent) : QComboBox(parent) {
	QObject::connect(this, SIGNAL(activated(int)), this, SLOT(redden(void)));
}

AxoPatchComboBox::~AxoPatchComboBox(void) {}

void AxoPatchComboBox::blacken(void) {
	palette.setColor(QPalette::Text, Qt::black);
	this->setPalette(palette);
}

void AxoPatchComboBox::redden(void) {
	palette.setColor(QPalette::Text, Qt::red);
	this->setPalette(palette);
}



AxoPatchSpinBox::AxoPatchSpinBox(QWidget *parent) : QSpinBox(parent) {
	QObject::connect(this, SIGNAL(valueChanged(int)), this, SLOT(redden(void)));
}

AxoPatchSpinBox::~AxoPatchSpinBox(void) {}

void AxoPatchSpinBox::blacken(void) {
	palette.setColor(QPalette::Text, Qt::black);
	this->setPalette(palette);
}

void AxoPatchSpinBox::redden(void) {
	palette.setColor(QPalette::Text, Qt::red);
	this->setPalette(palette);
}



extern "C" Plugin::Object * createRTXIPlugin(void) {
	return new AxoPatch();
};

static DefaultGUIModel::variable_t vars[] = {
	{ "Mode Telegraph", "", DefaultGUIModel::INPUT, },
	{ "Gain Telegraph", "", DefaultGUIModel::INPUT, },
	{ "Input Channel", "", DefaultGUIModel::PARAMETER | DefaultGUIModel::INTEGER, },
	{ "Output Channel", "", DefaultGUIModel::PARAMETER | DefaultGUIModel::INTEGER, },
	{ "Headstage Gain", "", DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
	{ "Output Gain", "", DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, }, 
	{ "Amplifier Mode", "", DefaultGUIModel::PARAMETER | DefaultGUIModel::INTEGER, },
};

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);

static void getDevice(DAQ::Device *d, void *p) {
	DAQ::Device **device = reinterpret_cast<DAQ::Device **>(p);

	if (!*device) *device = d;
}

AxoPatch::AxoPatch(void) : DefaultGUIModel("AxoPatch 200 Controller", ::vars, ::num_vars) {
	setWhatsThis("<p>Yeah, I'll get to this later... <br>-Ansel</p>");
	DefaultGUIModel::createGUI(vars, num_vars);
	initParameters();
	customizeGUI();
	update( INIT );
	DefaultGUIModel::refresh();
};

AxoPatch::~AxoPatch(void) {};

void AxoPatch::initParameters(void) {
	input_channel = 0;
	output_channel = 0;
	amp_mode = 1;
	output_gain = headstage_gain = 1;

	device = 0;
	DAQ::Manager::getInstance()->foreachDevice(getDevice, &device);

	iclamp_ai_gain = 1; // (1 V/V)
	iclamp_ao_gain = 1.0 / 2e-9; // (2 nA/V)
	izero_ai_gain = 1; // (1 V/V)
	izero_ao_gain = 0; // No output
	vclamp_ai_gain = 1e-12/1e-3; // (1 mV/pA)
	vclamp_ao_gain = 1 / 20e-3; // (20 mV/V)
};

void AxoPatch::update(DefaultGUIModel::update_flags_t flag) {
	switch(flag) {
		case INIT:
			setParameter("Input Channel", input_channel);
			setParameter("Output Channel", output_channel);
			setParameter("Headstage Gain", headstage_gain);
			setParameter("Output Gain", output_gain);
			setParameter("Amplifier Mode", amp_mode);

			updateGUI();
			break;
		
		case MODIFY:
			input_channel = getParameter("Input Channel").toInt();
			output_channel = getParameter("Output Channel").toInt();
			output_gain = getParameter("Output Gain").toDouble();
			headstage_gain = getParameter("Headstage Gain").toDouble();
			if (amp_mode != getParameter("Amplifier Mode").toInt()) {
				ampButtonGroup->button(amp_mode)->setStyleSheet("QRadioButton { font: normal; }");
				ampButtonGroup->button(getParameter("Amplifier Mode").toInt())->setStyleSheet("QRadioButton { font: bold;}");
				amp_mode = getParameter("Amplifier Mode").toInt();
			}

			updateDAQ();

			inputBox->blacken();
			outputBox->blacken();
			headstageBox->blacken();
			outputGainBox->blacken();
			break;

		case PAUSE:
			inputBox->setEnabled(true);
			outputBox->setEnabled(true);
			outputGainBox->setEnabled(true);
			headstageBox->setEnabled(true);
			iclampButton->setEnabled(true);
			vclampButton->setEnabled(true);
			modifyButton->setEnabled(true);
			break;

		case UNPAUSE:
			inputBox->setEnabled(false);
			outputBox->setEnabled(false);
			outputGainBox->setEnabled(false);
			headstageBox->setEnabled(false);
			iclampButton->setEnabled(false);
			vclampButton->setEnabled(false);
			modifyButton->setEnabled(false);
			break;

		default:
			break;
	}
}

void AxoPatch::execute(void) {
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

void AxoPatch::updateGUI(void) {
	inputBox->setValue(input_channel);
	outputBox->setValue(output_channel);

	if (headstage_gain == .1) {
		headstageBox->setCurrentIndex(2);
	} else {
		headstageBox->setCurrentIndex(0);
	}

	if (output_gain == .1) {
		outputGainBox->setCurrentIndex(0);
	} else if (output_gain == .2) {
		outputGainBox->setCurrentIndex(1);
	} else if (output_gain == .5) {
		outputGainBox->setCurrentIndex(2);
	} else if (output_gain == 1) {
		outputGainBox->setCurrentIndex(3);
	} else if (output_gain == 2) {
		outputGainBox->setCurrentIndex(4);
	} else if (output_gain == 5) {
		outputGainBox->setCurrentIndex(5);
	} else if (output_gain == 10) {
		outputGainBox->setCurrentIndex(6);
	} else if (output_gain == 20) {
		outputGainBox->setCurrentIndex(7);
	} else if (output_gain == 50) {
		outputGainBox->setCurrentIndex(8);
	} else if (output_gain == 100) {
		outputGainBox->setCurrentIndex(9);
	} else if (output_gain == 200) {
		outputGainBox->setCurrentIndex(10);
	} else if (output_gain == 500) {
		outputGainBox->setCurrentIndex(11);
	} else {
		outputGainBox->setCurrentIndex(0);
	}
	
	switch(amp_mode) {
		case 1:
			ampButtonGroup->button(1)->setChecked(true);
			ampButtonGroup->button(1)->setStyleSheet("QRadioButton { font: bold; }");
			ampButtonGroup->button(2)->setStyleSheet("QRadioButton { font: normal; }");
		case 2:
			ampButtonGroup->button(2)->setChecked(true);
			ampButtonGroup->button(2)->setStyleSheet("QRadioButton { font: bold; }");
			ampButtonGroup->button(1)->setStyleSheet("QRadioButton { font: normal; }");
		default:
			ampButtonGroup->button(1)->setChecked(true);
			ampButtonGroup->button(1)->setStyleSheet("QRadioButton { font: bold; }");
			ampButtonGroup->button(2)->setStyleSheet("QRadioButton { font: normal; }");
	}
}

void AxoPatch::updateMode(int value) {
	for (std::map<QString, param_t>::iterator i = parameter.begin(); i != parameter.end(); ++i) {
		if (i->first == "Amplifier Mode") {
			i->second.edit->setText(QString::number(value));
			i->second.edit->setModified(true);
			break;
		}
	}
}

void AxoPatch::updateOutputGain(int value) {
	double temp_value;

	switch(value) {
		case 0:
			temp_value = .1;
			break;
		case 1:
			temp_value = .2;
			break;
		case 2:
			temp_value = .5;
			break;
		case 3:
			temp_value = 1;
			break;
		case 4:
			temp_value = 2;
			break;
		case 5:
			temp_value = 5;
			break;
		case 6:
			temp_value = 10;
			break;
		case 7:
			temp_value = 20;
			break;
		case 8:
			temp_value = 50;
			break;
		case 9:
			temp_value = 100;
			break;
		case 10:
			temp_value = 200;
			break;
		case 11:
			temp_value = 500;
			break;
		default:
			temp_value = 0;
			break;
	}

	for (std::map<QString, param_t>::iterator i = parameter.begin(); i != parameter.end(); ++i) {
		if (i->first == "Output Gain") {
			i->second.edit->setText(QString::number(temp_value));
			i->second.edit->setModified(true);
			break;
		}
	}
}

void AxoPatch::updateHeadstageGain(int value) {
	double temp_value;

	switch(value) {
		case 2: 
			temp_value = .1;
			break;

		default:
			temp_value = 1;
			break;
	}

	for (std::map<QString, param_t>::iterator i = parameter.begin(); i != parameter.end(); ++i) {
		if (i->first == "Headstage Gain") {
			i->second.edit->setText(QString::number(temp_value));
			i->second.edit->setModified(true);
			break;
		}
	}
}

void AxoPatch::updateOutputChannel(int value) {
	for (std::map<QString, param_t>::iterator i = parameter.begin(); i != parameter.end(); ++i) {
		if (i->first == "Output Channel") {
			i->second.edit->setText(QString::number(value));
			i->second.edit->setModified(true);
			break;
		}
	}
}

void AxoPatch::updateInputChannel(int value) {
	for (std::map<QString, param_t>::iterator i = parameter.begin(); i != parameter.end(); ++i) {
		if (i->first == "Input Channel") {
			i->second.edit->setText(QString::number(value));
			i->second.edit->setModified(true);
			break;
		}
	}
}

void AxoPatch::updateDAQ(void) {
	if (!device) return;

	switch(amp_mode) {
		case 1: //IClamp
			device->setAnalogRange(DAQ::AI, input_channel, 0);
			std::cout<<"Flag1"<<std::endl;
			if (!getActive()) {
				device->setAnalogGain(DAQ::AI, input_channel, iclamp_ai_gain/output_gain*headstage_gain);
			} else {
				device->setAnalogGain(DAQ::AI, input_channel, iclamp_ai_gain/output_gain);
			}
			device->setAnalogGain(DAQ::AO, output_channel, iclamp_ao_gain*headstage_gain);
			device->setAnalogCalibration(DAQ::AO, output_channel);
			device->setAnalogCalibration(DAQ::AI, input_channel);
			break;

		case 2: //VClamp
			device->setAnalogRange(DAQ::AI, input_channel, 0);
			if (!getActive()) {
				device->setAnalogGain(DAQ::AI, input_channel, vclamp_ai_gain/output_gain*headstage_gain);
			} else {
				device->setAnalogGain(DAQ::AI, input_channel, vclamp_ai_gain/output_gain);
			}
			device->setAnalogGain(DAQ::AO, output_channel, vclamp_ao_gain);
			device->setAnalogCalibration(DAQ::AO, output_channel);
			device->setAnalogCalibration(DAQ::AI, input_channel);
			break;

		default:
			std::cout<<"ERROR. Something went horribly wrong.\n The amplifier mode is set to an unknown value"<<std::endl;
			break;
	}
};


void AxoPatch::customizeGUI(void) {
	QGridLayout *customLayout = DefaultGUIModel::getLayout();
	
	customLayout->itemAtPosition(1,0)->widget()->setVisible(false);
	DefaultGUIModel::pauseButton->setText("Auto");
	DefaultGUIModel::modifyButton->setText("Set DAQ");
	DefaultGUIModel::unloadButton->setVisible(false);

	// create input spinboxes
	QGroupBox *ioBox = new QGroupBox;
	QHBoxLayout *ioBoxLayout = new QHBoxLayout;
	ioBox->setLayout(ioBoxLayout);
	inputBox = new AxoPatchSpinBox;
	inputBox->setRange(0,100);
	outputBox = new AxoPatchSpinBox;//QSpinBox;
	outputBox->setRange(0,100);
	
	QLabel *inputBoxLabel = new QLabel("Input");
	QLabel *outputBoxLabel = new QLabel("Output");
	ioBoxLayout->addWidget(inputBoxLabel);
	ioBoxLayout->addWidget(inputBox);
	ioBoxLayout->addWidget(outputBoxLabel);
	ioBoxLayout->addWidget(outputBox);

	// create gain and amplifer mode comboboxes
	QGroupBox *comboBoxGroup = new QGroupBox;
	QFormLayout *comboBoxLayout = new QFormLayout;
	headstageBox = new AxoPatchComboBox;
	headstageBox->insertItem( 0, trUtf8("\x50\x61\x74\x63\x68\x20\xce\xb2\x3d\x31") );
	headstageBox->insertItem( 1, trUtf8( "\x57\x68\x6f\x6c\x65\x20\x43\x65\x6c\x6c\x20\xce\xb2\x3d\x31" ) );
	headstageBox->insertItem( 2, trUtf8( "\x57\x68\x6f\x6c\x65\x20\x43\x65\x6c\x6c\x20\xce\xb2\x3d\x30\x2e\x31" ) );
	outputGainBox = new AxoPatchComboBox;
	outputGainBox->insertItem( 0, tr("0.1") );
	outputGainBox->insertItem( 1, tr("0.2") );
	outputGainBox->insertItem( 2, tr("0.5") );
	outputGainBox->insertItem( 3, tr("1") );
	outputGainBox->insertItem( 4, tr("2") );
	outputGainBox->insertItem( 5, tr("5") );
	outputGainBox->insertItem( 6, tr("10") );
	outputGainBox->insertItem( 7, tr("20") );
	outputGainBox->insertItem( 8, tr("50") );
	outputGainBox->insertItem( 9, tr("100") );
	outputGainBox->insertItem( 10, tr("200") );
	outputGainBox->insertItem( 11, tr("500") );
	comboBoxLayout->addRow("Output Gain", outputGainBox);
	comboBoxLayout->addRow("Headstage", headstageBox);
	comboBoxGroup->setLayout(comboBoxLayout);

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
	ampModeLabel = new QLabel;
	ampModeLabel->setText("Amp Mode");
	ampModeLabel->setAlignment(Qt::AlignCenter);

	QVBoxLayout *col1Layout = new QVBoxLayout;
	col1Layout->addWidget(ampModeLabel);
	ampModeBoxLayout->addLayout(col1Layout);
	
	QVBoxLayout *col2Layout = new QVBoxLayout;
	col2Layout->addWidget(iclampButton);
	col2Layout->addWidget(vclampButton);
	ampButtonGroup->setExclusive(true);
	ampModeBoxLayout->addLayout(col2Layout);

	// add widgets to custom layout
	customLayout->addWidget(ioBox, 0, 0);
	customLayout->addWidget(comboBoxGroup, 2, 0);
	customLayout->addWidget(ampModeBox, 3, 0);
	setLayout(customLayout);

	QObject::connect(ampButtonGroup, SIGNAL(buttonPressed(int)), this, SLOT(updateMode(int)));
	QObject::connect(outputGainBox, SIGNAL(activated(int)), this, SLOT(updateOutputGain(int)));
	QObject::connect(headstageBox, SIGNAL(activated(int)), this, SLOT(updateHeadstageGain(int)));
	QObject::connect(inputBox, SIGNAL(valueChanged(int)), this, SLOT(updateInputChannel(int)));
	QObject::connect(outputBox, SIGNAL(valueChanged(int)), this, SLOT(updateOutputChannel(int)));
}

void AxoPatch::refresh(void) {
	for (std::map<QString, param_t>::iterator i = parameter.begin(); i!= parameter.end(); ++i) {
		if (i->second.type & (STATE | EVENT)) {
			i->second.edit->setText(QString::number(getValue(i->second.type, i->second.index)));
			palette.setBrush(i->second.edit->foregroundRole(), Qt::darkGray);
			i->second.edit->setPalette(palette);
		} else if ((i->second.type & PARAMETER) && !i->second.edit->isModified() && i->second.edit->text() != *i->second.str_value) {
			i->second.edit->setText(*i->second.str_value);
		} else if ((i->second.type & COMMENT) && !i->second.edit->isModified() && i->second.edit->text() != QString::fromStdString(getValueString(COMMENT, i->second.index))) {
			i->second.edit->setText(QString::fromStdString(getValueString(COMMENT, i->second.index)));
		}
	}
	pauseButton->setChecked(!getActive());

	if (getActive()) {
		switch(amp_mode) {
			case 1:
				ampModeLabel->setText("IClamp");
				break;
			case 2:
				ampModeLabel->setText("VClamp");
				break;
			default:
				break;
		}
	}
	
	if (settings_changed) {
		updateDAQ();
		settings_changed = false;
	}
}

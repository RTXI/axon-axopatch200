#include <QtGui>
#include "axon-axopatch-200.h"
#include <iostream>

extern "C" Plugin::Object * createRTXIPlugin(void) {
	return new AxoPatch();
};

static DefaultGUIModel::variable_t vars[] = {
	{ "Mode Telegraph", "", DefaultGUIModel::INPUT, },
	{ "Gain Telegraph", "", DefaultGUIModel::INPUT, },
	{ "Input Channel", "", DefaultGUIModel::PARAMETER | DefaultGUIModel::UINTEGER, },
	{ "Output Channel", "", DefaultGUIModel::PARAMETER | DefaultGUIModel::UINTEGER, },
	{ "Headstage Configuration", "", DefaultGUIModel::PARAMETER | DefaultGUIModel::UINTEGER, }, 
	{ "Amplifier Mode", "", DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
};

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);

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

	vclamp_gain = iclamp_gain = 1;
};

void AxoPatch::update(DefaultGUIModel::update_flags_t flag) {
	switch(flag) {
		case INIT:
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

void AxoPatch::customizeGUI(void) {
	QGridLayout *customLayout = DefaultGUIModel::getLayout();
	
	customLayout->itemAtPosition(1,0)->widget()->setVisible(false);
	customLayout->itemAtPosition(10,0)->widget()->setVisible(false);

	// create input spinboxes
	QHBoxLayout *ioBoxLayout = new QHBoxLayout;
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
	headstageBox->addItem( trUtf8("\x50\x61\x74\x63\x68\x20\xce\xb2\x3d\x31") );
	headstageBox->addItem( trUtf8( "\x57\x68\x6f\x6c\x65\x20\x43\x65\x6c\x6c\x20\xce\xb2\x3d\x31" ) );
	headstageBox->addItem( trUtf8( "\x57\x68\x6f\x6c\x65\x20\x43\x65\x6c\x6c\x20\xce\xb2\x3d\x30\x2e\x31" ) );
	outputGainBox = new QComboBox;
	outputGainBox->addItem( tr("0.5") );
	outputGainBox->addItem( tr("1") );
	outputGainBox->addItem( tr("2") );
	outputGainBox->addItem( tr("5") );
	outputGainBox->addItem( tr("10") );
	outputGainBox->addItem( tr("20") );
	outputGainBox->addItem( tr("50") );
	outputGainBox->addItem( tr("100") );
	outputGainBox->addItem( tr("200") );
	outputGainBox->addItem( tr("500") );
	comboBoxLayout->addRow("Output Gain", outputGainBox);
	comboBoxLayout->addRow("Headstg Config", headstageBox);

	// create amp mode groupbox
	QGroupBox *ampModeBox = new QGroupBox("Amplifier Mode");
	QVBoxLayout *ampModeBoxLayout = new QVBoxLayout;
	ampModeBox->setLayout(ampModeBoxLayout);
	QButtonGroup *ampButtonGroup = new QButtonGroup;
	iclampButton = new QPushButton("IClamp");
	iclampButton->setCheckable(true);
	ampButtonGroup->addButton(iclampButton);
	vclampButton = new QPushButton("VClamp");
	vclampButton->setCheckable(true);
	ampButtonGroup->addButton(vclampButton);
	autoButton = new QPushButton("Auto");
	QHBoxLayout *rowLayout = new QHBoxLayout;
	rowLayout->addWidget(iclampButton);
	rowLayout->addWidget(vclampButton);
	ampButtonGroup->setExclusive(true);
	ampModeBoxLayout->addLayout(rowLayout);
	ampModeBoxLayout->addWidget(autoButton);

	// add separator line(s)
	QFrame *line1 = new QFrame;
	line1->setFrameShape(QFrame::HLine);
	line1->setFrameShadow(QFrame::Sunken);
	QFrame *line2 = new QFrame;
	line2->setFrameShape(QFrame::HLine);
	line2->setFrameShadow(QFrame::Sunken);

	// add widgets to custom layout
	customLayout->addLayout(ioBoxLayout, 0, 0);
	customLayout->addWidget(line1, 2, 0);
	customLayout->addLayout(comboBoxLayout, 3, 0);
	customLayout->addWidget(line2, 4, 0);
	customLayout->addWidget(ampModeBox, 5, 0);
	setLayout(customLayout);
}

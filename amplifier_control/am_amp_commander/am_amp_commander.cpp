/*
 * Copyright (C) 2008 Georgia Institute of Technology
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <am_amp_commander.h>
#include <main_window.h>
#include <rtfile.h>
#include <math.h>
#include <algorithm>
#include <time.h>

#include <qhbox.h>
#include <qhbuttongroup.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpicture.h>
#include <qprinter.h>
#include <qprintdialog.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qvalidator.h>
#include <qvbox.h>
#include <qwhatsthis.h>

/* DO NOT EDIT */
namespace {

class SyncEvent: public RT::Event {

public:

	int callback(void) {
		return 0;
	}
	;

}; // class SyncEvent

} // namespace
/* END DO NOT EDIT */

extern "C" Plugin::Object *
createRTXIPlugin(void) {
	return new AMAmpCommander(); // Change the name of the plug-in here
}

// Specify inputs, outputs, and parameters here to construct labels and textboxes a la default_gui_model
static AMAmpCommander::variable_t vars[] = {
		{ "Input Mode","Vclamp | I == 0 | Iclamp", AMAmpCommander::INPUT | AMAmpCommander::INTEGER, },
		{ "Mode Bit 1", "Mode Bit 1", AMAmpCommander::OUTPUT, },
		{ "Mode Bit 2", "Mode Bit 2", AMAmpCommander::OUTPUT, },
		{ "Mode Bit 4", "Mode Bit 4", AMAmpCommander::OUTPUT, },
};

// This is a necessary variable, change the scope
static size_t num_vars = sizeof(vars) / sizeof(AMAmpCommander::variable_t);

static void getDevice(DAQ::Device *d,void *p) {
    DAQ::Device **device = reinterpret_cast<DAQ::Device **>(p);

    if(!*device)
        *device = d;
}

// Default constructor
AMAmpCommander::AMAmpCommander(void) :
	QWidget(MainWindow::getInstance()->centralWidget()), Workspace::Instance(
			"AM Amp Commander", vars, num_vars) {
	QWhatsThis::add(this, "<p><b>AM Amp Commander</b></p>");
	setCaption(QString::number(getID()) + " AM Amp Commander");

	QBoxLayout *layout = new QVBoxLayout(this); // overall GUI layout

	QHButtonGroup *modeBox = new QHButtonGroup("AMAmpCommander Mode", this);
	modeBox->setRadioButtonExclusive(true);
	vclampButton = new QRadioButton("Vclamp", modeBox);
	izeroButton = new QRadioButton("Izero", modeBox);
	iclampButton = new QRadioButton("IClamp", modeBox);
	izeroButton->setChecked(true);
	//QObject::connect(modeBox,SIGNAL(clicked(int)),this,SLOT(setMode(int)));

	QHBox *utilityBox = new QHBox(this);
	pauseButton = new QPushButton("Pause", utilityBox);
	pauseButton->setToggleButton(true);
	QObject::connect(pauseButton,SIGNAL(toggled(bool)),this,SLOT(pause(bool)));
	QPushButton *modifyButton = new QPushButton("Modify", utilityBox);
	QObject::connect(modifyButton,SIGNAL(clicked(void)),this,SLOT(modify(void)));
	QPushButton *unloadButton = new QPushButton("Unload", utilityBox);
	QObject::connect(unloadButton,SIGNAL(clicked(void)),this,SLOT(exit(void)));
	QObject::connect(pauseButton,SIGNAL(toggled(bool)),modifyButton,SLOT(setEnabled(bool)));
	QToolTip::add(pauseButton, "Start/Stop current AMAmpCommander protocol");
	QToolTip::add(unloadButton, "Close plugin");

	// Add custom left side GUI components to layout above default_gui_model components
	layout->addWidget(modeBox);

	// Create default_gui_model GUI DO NOT EDIT
	QScrollView *sv = new QScrollView(this);
	sv->setResizePolicy(QScrollView::AutoOneFit);
	layout->addWidget(sv);

	QWidget *viewport = new QWidget(sv->viewport());
	sv->addChild(viewport);
	QGridLayout *scrollLayout = new QGridLayout(viewport, 1, 2);

	size_t nstate = 0, nparam = 0, nevent = 0;
	for (size_t i = 0; i < num_vars; i++) {
		if (vars[i].flags & (PARAMETER | STATE | EVENT)) {
			param_t param;
			param.label = new QLabel(vars[i].name, viewport);
			param.label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
			scrollLayout->addWidget(param.label, parameter.size(), 0);
			param.edit = new DefaultGUILineEdit(viewport);
			param.edit->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
			scrollLayout->addWidget(param.edit, parameter.size(), 1);
			QToolTip::add(param.label, vars[i].description);
			QToolTip::add(param.edit, vars[i].description);

			if (vars[i].flags & PARAMETER) {
				if (vars[i].flags & DOUBLE) {
					param.edit->setValidator(new QDoubleValidator(param.edit));
					param.type = PARAMETER | DOUBLE;
				} else if (vars[i].flags & UINTEGER) {
					QIntValidator *validator = new QIntValidator(param.edit);
					param.edit->setValidator(validator);
					validator->setBottom(0);
					param.type = PARAMETER | UINTEGER;
				} else if (vars[i].flags & INTEGER) {
					param.edit->setValidator(new QIntValidator(param.edit));
					param.type = PARAMETER | INTEGER;
				} else
					param.type = PARAMETER;
				param.index = nparam++;
				param.str_value = new QString;
				setData(PARAMETER, param.index, param.value = new double);
			} else {
				if (vars[i].flags & STATE) {
					param.edit->setReadOnly(true);
					param.type = STATE;
					param.index = nstate++;
				} else {
					if (vars[i].flags & EVENT) {
						param.edit->setReadOnly(true);
						param.type = EVENT;
						param.index = nevent++;
					}
				}
			}
			parameter[vars[i].name] = param;
		}
	}
	// end default_gui_model GUI DO NOT EDIT

	// add custom components to layout below default_gui_model components
	layout->addWidget(utilityBox);
	layout->setResizeMode(QLayout::Fixed);

	// set GUI refresh rate
	QTimer *timer = new QTimer(this);
	timer->start(1000);
	show();

	// initialize
    DAQ::Manager::getInstance()->foreachDevice(getDevice,&device);

	update(INIT);
	refresh(); // refresh the GUI
	printf("\nStarting AM Amp Commander:\n"); // prints to terminal
}

AMAmpCommander::~AMAmpCommander(void) {
}

void AMAmpCommander::execute(void) {

	setMode(static_cast<ampmode_t>(input(0)));
	return;
}

void AMAmpCommander::update(AMAmpCommander::update_flags_t flag) {
	switch (flag) {
	case INIT:
		initParameters();
	    setMode(Izero);
	    break;
	case MODIFY:
		break;
	case PAUSE:
		setMode(Izero);
		break;
	case UNPAUSE:
		break;
	case PERIOD:
		break;
	case EXIT:
		printf("\nStopping AM Amp Commander:\n"); // prints to terminal
	default:
		break;
	}
}

// custom functions 

void AMAmpCommander::initParameters() {
	iclamp_ai_gain = 200e-3;
	iclamp_ao_gain = 2e-9;
	izero_ai_gain = 200e-3;
	izero_ao_gain = 1;
	vclamp_ai_gain = 2e-9;
	vclamp_ao_gain = 20e-3;
	device = 0;
}

void AMAmpCommander::setMode(ampmode_t mode) {

    this->mode = mode;

    if(mode == Iclamp) {
        if(device) {
            device->setAnalogGain(DAQ::AI,0,iclamp_ai_gain);
            device->setAnalogGain(DAQ::AO,0,iclamp_ao_gain);
        }
        output(0) = 0.0;
        output(1) = 0.0;
        output(2) = 5.0;
        iclampButton->setChecked(true);
    } else if(mode == Vclamp) {
        if(device) {
            device->setAnalogGain(DAQ::AI,0,vclamp_ai_gain);
            device->setAnalogGain(DAQ::AO,0,vclamp_ao_gain);
        }
        output(0) = 0.0;
        output(1) = 5.0;
        output(2) = 0.0;
        vclampButton->setChecked(true);
    } else {
        /* Izero */
        if(device) {
            device->setAnalogGain(DAQ::AI,0,izero_ai_gain);
            device->setAnalogGain(DAQ::AO,0,izero_ao_gain);
        }
        output(0) = 5.0;
        output(1) = 5.0;
        output(2) = 0.0;
        izeroButton->setChecked(true);
    }
}

// required functions from default_gui_model DO NOT EDIT

void AMAmpCommander::exit(void) {
	update(EXIT);
	Plugin::Manager::getInstance()->unload(this);

}

void AMAmpCommander::refresh(void) {
	for (std::map<QString, param_t>::iterator i = parameter.begin(); i
			!= parameter.end(); ++i) {
		if (i->second.type & (STATE | EVENT))
			i->second.edit->setText(QString::number(getValue(i->second.type,
					i->second.index)));
		else if ((i->second.type & PARAMETER) && !i->second.edit->edited()
				&& i->second.edit->text() != *i->second.str_value)
			i->second.edit->setText(*i->second.str_value);
	}
	pauseButton->setOn(!getActive());
}

void AMAmpCommander::modify(void) {
	bool active = getActive();
	setActive(false);
	// Ensure that the realtime thread isn't in the middle of executing AMAmpCommander::execute()
	SyncEvent event;
	RT::System::getInstance()->postEvent(&event);
	update(MODIFY);
	setActive(active);
	for (std::map<QString, param_t>::iterator i = parameter.begin(); i
			!= parameter.end(); ++i)
		i->second.edit->blacken();
}

void AMAmpCommander::pause(bool p) {
	if (pauseButton->isOn() != p)
		pauseButton->setDown(p);
	setActive(!p);
	if (p)
		update(PAUSE);
	else
		update(UNPAUSE);
}

QString AMAmpCommander::getParameter(const QString &name) {
	std::map<QString, param_t>::iterator n = parameter.find(name);
	if ((n != parameter.end()) && (n->second.type & PARAMETER)) {
		*n->second.str_value = n->second.edit->text();
		*n->second.value = n->second.edit->text().toDouble();
		return n->second.edit->text();
	}
	return "";
}

void AMAmpCommander::setParameter(const QString &name, double value) {
	std::map<QString, param_t>::iterator n = parameter.find(name);
	if ((n != parameter.end()) && (n->second.type & PARAMETER)) {
		n->second.edit->setText(QString::number(value));
		*n->second.str_value = n->second.edit->text();
		*n->second.value = n->second.edit->text().toDouble();
	}
}

void AMAmpCommander::setParameter(const QString &name, const QString value) {
	std::map<QString, param_t>::iterator n = parameter.find(name);
	if ((n != parameter.end()) && (n->second.type & PARAMETER)) {
		n->second.edit->setText(value);
		*n->second.str_value = n->second.edit->text();
		*n->second.value = n->second.edit->text().toDouble();
	}
}

void AMAmpCommander::setState(const QString &name, double &ref) {
	std::map<QString, param_t>::iterator n = parameter.find(name);
	if ((n != parameter.end()) && (n->second.type & STATE)) {
		setData(Workspace::STATE, n->second.index, &ref);
		n->second.edit->setText(QString::number(ref));
	}
}

void AMAmpCommander::setEvent(const QString &name, double &ref) {
	std::map<QString, param_t>::iterator n = parameter.find(name);
	if ((n != parameter.end()) && (n->second.type & EVENT)) {
		setData(Workspace::EVENT, n->second.index, &ref);
		n->second.edit->setText(QString::number(ref));
	}
}

void AMAmpCommander::doLoad(const Settings::Object::State &s) {
	for (std::map<QString, param_t>::iterator i = parameter.begin(); i
			!= parameter.end(); ++i)
		i->second.edit->setText(s.loadString(i->first));
	pauseButton->setOn(s.loadInteger("paused"));
	modify();
}

void AMAmpCommander::doSave(Settings::Object::State &s) const {
	s.saveInteger("paused", pauseButton->isOn());
	for (std::map<QString, param_t>::const_iterator i = parameter.begin(); i
			!= parameter.end(); ++i)
		s.saveString(i->first, i->second.edit->text());
}
/* compatible with RTXI v1.1
 void AMAmpCommander::receiveEvent(const Event::Object *event) {
 if(event->getName() == RT::System::PRE_PERIOD_EVENT) {
 periodEventPaused = getActive();
 setActive(false);
 } else if(event->getName() == RT::System::POST_PERIOD_EVENT) {
 #ifdef DEBUG
 if(getActive())
 ERROR_MSG("AMAmpCommander::receiveEvent : model unpaused during a period update\n");
 #endif
 update(PERIOD);
 setActive(periodEventPaused);
 }
 }
 */

/* compatible with RTXI v1.1.2 */
void AMAmpCommander::receiveEvent(const Event::Object *event) {
	if (event->getName() == Event::RT_PREPERIOD_EVENT) {
		periodEventPaused = getActive();
		setActive(false);
	} else if (event->getName() == Event::RT_POSTPERIOD_EVENT) {
#ifdef DEBUG
		if(getActive())
		ERROR_MSG("DefaultGUIModel::receiveEvent : model unpaused during a period update\n");
#endif
		update(PERIOD);
		setActive(periodEventPaused);
	}
}


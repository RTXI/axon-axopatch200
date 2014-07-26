#ifndef AM_AMP_COMMANDER_H
#define AM_AMP_COMMANDER_H

#include <daq.h>
#include <qobject.h>
#include <qwidget.h>
#include <event.h>
#include <map>
#include <mutex.h>
#include <plugin.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <rt.h>
#include <workspace.h>
#include <default_gui_model.h>
#include <qstring.h>
#include <cstdlib>
#include <qradiobutton.h>

using namespace std;

class QLabel;
class QPushButton;

class AMAmpCommander: public QWidget,
		public RT::Thread,
		public Plugin::Object,
		public Workspace::Instance,
		public Event::Handler {

Q_OBJECT

public:

	/* DO NOT EDIT From default_gui_model */
	static const IO::flags_t INPUT = Workspace::INPUT;
	static const IO::flags_t OUTPUT = Workspace::OUTPUT;
	static const IO::flags_t PARAMETER = Workspace::PARAMETER;
	static const IO::flags_t STATE = Workspace::STATE;
	static const IO::flags_t EVENT = Workspace::EVENT;
	static const IO::flags_t DOUBLE = Workspace::EVENT << 1;
	static const IO::flags_t INTEGER = Workspace::EVENT << 2;
	static const IO::flags_t UINTEGER = Workspace::EVENT << 3;
	typedef Workspace::variable_t variable_t;
	/* END DO NOT EDIT */

	enum update_flags_t {
		INIT, /*!< The parameters need to be initialized.         */
		MODIFY, /*!< The parameters have been modified by the user. */
		PERIOD, /*!< The system period has changed.                 */
		PAUSE, /*!< The Pause button has been activated            */
		UNPAUSE, /*!< When the pause button has been deactivated     */
		EXIT,
	// add whatever additional flags you want here
	};

	// Custom flags for AMAmpCommander mode
	enum ampmode_t {
		Izero, Vclamp, Iclamp,
	};

	AMAmpCommander(void);
	virtual
	~AMAmpCommander(void);
	void
	execute(void);

	void setMode(ampmode_t);

public slots:
	/* DO NOT EDIT */
	void exit(void);
	void
	refresh(void);
	void
	modify(void);
	void
	pause(bool);
	/* END DO NOT EDIT */

	signals: // all custom signals

protected:
	// Apart from changing the scope for update_flags_t, do not edit these
	void
	update(AMAmpCommander::update_flags_t); // run each time model parameters are updated
	QString
	getParameter(const QString &name);
	void
	setParameter(const QString &name, double value);
	void
	setParameter(const QString &name, const QString value);
	void
	setState(const QString &name, double &ref);
	void
	setEvent(const QString &name, double &ref);

private:
	void setIclamp(void);
	void setVclamp(void);

	DAQ::Device *device;

	double iclamp_ai_gain;
	double iclamp_ao_gain;
	double izero_ai_gain;
	double izero_ao_gain;
	double vclamp_ai_gain;
	double vclamp_ao_gain;
	ampmode_t mode;

	QPushButton *pauseButton;
	QRadioButton *vclampButton;
	QRadioButton *iclampButton;
	QRadioButton *izeroButton;

	// AMAmpCommander functions
	void
	initParameters();

	/* DO NOT EDIT */
	void
	doLoad(const Settings::Object::State &);
	void
	doSave(Settings::Object::State &) const;
	void
	receiveEvent(const Event::Object *);
	struct param_t {
		QLabel *label;
		DefaultGUILineEdit *edit;
		IO::flags_t type;
		size_t index;
		QString *str_value;
		double *value;
	};
	bool periodEventPaused;
	mutable QString junk;
	std::map<QString, param_t> parameter;
	/* END DO NOT EDIT */

private slots: // all custom slots

};

#endif // AM_AMP_COMMANDER_H

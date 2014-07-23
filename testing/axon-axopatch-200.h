#include <QtGui>
#include <daq.h>
#include <default_gui_model.h>

class AxoPatch : public DefaultGUIModel {
	
	Q_OBJECT
	
	public:
		AxoPatch(void);
		virtual ~AxoPatch(void);
	
		void initParameters(void);
		void customizeGUI(void);
		void execute(void);
		void updateDAQ(void);
	
	protected:
		virtual void update(DefaultGUIModel::update_flags_t);

	private:
		double iclamp_ai_gain; // (1 V / V)
		double iclamp_ao_gain; // (2 nA / V) ...hmm
		double izero_ai_gain; // (1 V / V)
		double izero_ao_gain; // No output
		double vclamp_ai_gain; // 1 mV / pA
		double vclamp_ao_gain; // 20 mV / V
//		const double iclamp_out_gain;
//		const double vclamp_out_gain;

		int input_channel, output_channel;
		int output_ui_index;
		double output_gain, temp_gain, headstage_gain;
		/**/double scaled_gain;
		int headstage_config;
		int amp_mode, temp_mode;
		
		bool auto_on, settings_changed;

		DAQ::Device *device;
	
		QPushButton /* *iclampButton, *vclampButton,*/ *autoButton;
		QRadioButton *iclampButton, *vclampButton;
		QButtonGroup *ampButtonGroup;
		QSpinBox *inputBox, *outputBox;
		QComboBox *headstageBox, *outputGainBox;
		QLabel *ampModeLabel;

	private slots:
		void updateMode(int);
		void updateOutputGain(int);
		void updateHeadstageBox(int);
		void updateInputChannel(int);
		void updateOutputChannel(int);
		void toggleAutoMode(bool);
};

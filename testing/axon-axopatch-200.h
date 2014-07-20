#include <QtGui>
#include <default_gui_model.h>

class AxoPatch : public DefaultGUIModel {
	
	Q_OBJECT
	
	public:
		AxoPatch(void);
		virtual ~AxoPatch(void);
	
		void initParameters(void);
		void customizeGUI(void);
	
	protected:
		virtual void update(DefaultGUIModel::update_flags_t);

	private:
		enum AmpMode_t {
			ICLAMP = 0,
			VCLAMP = 2,
		};
/*
		enum AmpGain_t {
			Ghalf = .5,
			G1 = 1,
			G2 = 2,
			G5 = 5,
			G10 = 10,
			G20 = 20,
			G50 = 50,
			G100 = 100,
			G200 = 200,
			G500 = 500
		};
*/
	
		int input_channel, output_channel;
		double vclamp_gain, iclamp_gain;
	
		QPushButton *iclampButton, *vclampButton, *autoButton;
		QSpinBox *inputBox, *outputBox;
		QComboBox *headstageBox, *outputGainBox;
};

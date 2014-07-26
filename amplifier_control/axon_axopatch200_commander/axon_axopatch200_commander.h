#ifndef AXON_AXOPATCH200_COMMANDER_H
#define AXON_AXOPATCH200_COMMANDER_H


#include <rt.h>
#include <settings.h>
#include <daq.h>
#include <io.h>
#include <plugin.h>
#include <axon_axopatch200_commanderUI.h>

#include <qvbuttongroup.h>
#include <qwidget.h>
#include <qcombobox.h>
#include <qtimer.h>

namespace Axon200Commander {

    class Amplifier {
        
    public:

        enum mode_t {
            Vclamp,
            Iclamp,
        };

        Amplifier( void );
        ~Amplifier( void );

        void setMode( mode_t, int, int );
        void setGains( double, double );
        mode_t getMode();

    private:

        DAQ::Device *device;

        double iclamp_ai_gain;
        double iclamp_ao_gain;
        double izero_ai_gain;
        double izero_ao_gain;
        double vclamp_ai_gain;
        double vclamp_ao_gain;
        double iclamp_out_gain;
        double vclamp_out_gain;
        double headstage_gain, scaled_gain;
        mode_t mode;

    }; // class Amplifier
    
    class Panel : public QWidget, public RT::Thread, virtual public Settings::Object, public IO::Block {
        Q_OBJECT

    public:

        Panel( QWidget * );
        ~Panel( void );
        void execute();

        QVButtonGroup *buttonGroup;
        QComboBox *ogComboBox;
        QComboBox *hcComboBox;
        QTimer *refreshTimer;
        Axon_200_Amp_CommanderUI *mainUI;        
        Amplifier amp;

    public slots:
        void updateMode( int );
        void updateGains( void );
        void toggleAutoMode( bool );
        void updateChannels( void );
        void refresh( void );

    private:
        virtual void doLoad( const Settings::Object::State & );
        virtual void doSave( Settings::Object::State & ) const;

        int inputChan;
        int outputChan;
        Amplifier::mode_t currentMode;
        double currentGain;
        double oldGain;

    }; // class Panel
    
    class Plugin : public QObject, public ::Plugin::Object {

        Q_OBJECT

        public:

        static Plugin *getInstance(void);

        //   Amplifier *getAmplifier(size_t =0);

    public slots:

        void createAxon200CommanderPanel( void );
        void removeAxon200CommanderPanel( Panel * );

    protected:

        virtual void doLoad( const Settings::Object::State & );
        virtual void doSave( Settings::Object::State & ) const;

    private:

        Plugin( void );
        ~Plugin( void );
        Plugin( const Plugin & ) {};
        Plugin &operator=( const Plugin & ) { return *getInstance(); };

        static Plugin *instance;        

        int menuID;
        std::list<Panel *> panelList;

    }; // class Plugin

}; // namespace Axon200Commander

#endif // AXON_AXOPATCH200_COMMANDER_H

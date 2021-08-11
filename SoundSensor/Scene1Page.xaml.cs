using System;
using Tizen.NUI;
using Tizen.NUI.BaseComponents;
using Tizen.NUI.Components;

namespace SoundSensor
{
    public partial class Scene1Page : View
    {
        private Timer myTimer;

        Adc_MCP3008 adcDevice;
        public Scene1Page()
        {
            InitializeComponent();
        }
        
        private bool getValue(ref uint value)
        {
            uint read_value = 0;
            if (!adcDevice.read(0, ref read_value))
                return false;
            Logger.print_bar(read_value);
            Logger.Debug($"sensor value - [{read_value}]");

            value = read_value;
            return true;
        }

        private void ButtonStart(object sender, ClickedEventArgs e)
        {
            DataUpdateStart();
        }

        private void ButtonStop(object sender, ClickedEventArgs e)
        {
            DataUpdateStop();
        }
        private void DataUpdateStart()
        {
            // Open device
            Logger.Debug("Open device");
            try
            {
                adcDevice = new Adc_MCP3008();
            }
            catch (Exception ex)
            {
                Logger.Error("Exception : ", ex.Message);
            }

            // Create timer
            Logger.Debug("Create timer");
            myTimer = new Timer(50);
            myTimer.Tick += TickEvent;
            myTimer.Start();
        }

        private bool TickEvent(object source, Tizen.NUI.Timer.TickEventArgs e)
        {
            uint value = 0;
            bool result = getValue(ref value);
            if (result) {
                Logger.Debug($"sensor value - [{value}]");
                SoundValue.Text = value.ToString();
            }
            else {
                Logger.Error("Fail to read value");
            }
            return true;
        }

        public void DataUpdateStop()
        {
            myTimer.Dispose();
            adcDevice.close();

            SoundValue.Text = "0";
        }
    }
}


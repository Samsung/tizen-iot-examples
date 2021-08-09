using System;
using Tizen.NUI;
using Tizen.NUI.BaseComponents;
using Tizen.NUI.Components;
using Tizen.Peripheral.I2c;

namespace AccelerometerI2C
{
    public partial class Scene1Page : View
    {
        private const int Bus = 0x01;
        private const int Address = 0x53;

        private const byte POWER_CTL_REGISTER = 0x2D;
        private const byte MEASURE_VALUE = 0x08;

        private const byte DATAX0_REGISTER = 0x32;

        private I2cDevice i2cDevice;
        private Timer myTimer;

        public Scene1Page()
        {
            InitializeComponent();
        }

        private void ButtonStart(object sender, ClickedEventArgs e)
        {
            // Start data measurement
            DataUpdateStart();
        }

        private void ButtonStop(object sender, ClickedEventArgs e)
        {
            // Stop data measurement
            DataUpdateStop();

            myAccelValueX.Text = "X:000 (0.00G)";
            myAccelValueY.Text = "Y:000 (0.00G)";
            myAccelValueZ.Text = "Z:000 (0.00G)";
        }

        private void DataUpdateStart()
        {
            // Open device
            Logger.Debug("Open device");
            i2cDevice = new I2cDevice(Bus, Address);

            // Start measurement
            try
            {
                Logger.Debug("Start measurement");
                i2cDevice.WriteRegisterByte(POWER_CTL_REGISTER, MEASURE_VALUE);
            }
            catch (Exception ex)
            {
                Logger.Error("Failed to WriteRegisterByte");
            }

            // Create timer
            Logger.Debug("Create timer");
            myTimer = new Timer(300);
            myTimer.Tick += TickEvent;
            myTimer.Start();
        }

        private (short x, short y, short z) GetAcceleration()
        {
            const int Length = 3;
            short[] rawData = new short[Length];

            for (int i = 0; i < Length; ++i)
            {
                byte register = (byte)(DATAX0_REGISTER + i * 2);
                ushort word = i2cDevice.ReadRegisterWord(register);
                rawData[i] = (short)word;
            }

            return (rawData[0], rawData[1], rawData[2]);
        }

        private bool TickEvent(object source, Tizen.NUI.Timer.TickEventArgs e)
        {
            (short x, short y, short z) = GetAcceleration();

            float cal_x = (float)x / 256;
            float cal_y = (float)y / 256;
            float cal_z = (float)z / 256;

            Logger.Debug($"Acceleration\tx={x:000},\ty={y:000},\tz={z:000}");
            Logger.Debug($"Acceleration\tx={cal_x:0.000},\ty={cal_y:0.000},\tz={cal_z:0.000}");

            myAccelValueX.Text = String.Format("X:{0:000} ({1:0.00}G)", x, cal_x);
            myAccelValueY.Text = String.Format("Y:{0:000} ({1:0.00}G)", y, cal_y);
            myAccelValueZ.Text = String.Format("Z:{0:000} ({1:0.00}G)", z, cal_z);

            return true;
        }

        public void DataUpdateStop()
        {
            myTimer.Dispose();
            i2cDevice.Close();
        }
    }
}

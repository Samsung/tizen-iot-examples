using System;
using Tizen.NUI;

namespace AccelerometerI2C
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            Accelerometer Instance = new Accelerometer();
            Instance.Run(args);
        }
    }
}

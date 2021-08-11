using System;
using Tizen.NUI;

namespace SoundSensor
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            Scene1 Instance = new Scene1();
            Instance.Run(args);
        }
    }
}

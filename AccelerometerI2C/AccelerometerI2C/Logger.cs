using System.Runtime.CompilerServices;

namespace AccelerometerI2C
{
    public static class Logger
    {
        private const string LOGTAG = "Accel";
        public static void Debug(string message, [CallerFilePath] string file = "", [CallerMemberName] string func = "", [CallerLineNumber] int line = 0)
        {
            global::Tizen.Log.Debug(LOGTAG, message, file, func, line);
        }
        public static void Error(string message, [CallerFilePath] string file = "", [CallerMemberName] string func = "", [CallerLineNumber] int line = 0)
        {
            global::Tizen.Log.Error(LOGTAG, message, file, func, line);

        }
    }
}

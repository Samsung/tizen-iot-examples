using Tizen.Applications;
using Tizen.NUI;


namespace AccelerometerI2C
{
    public class Accelerometer : NUIApplication
    {
        private MainPage mainPage;
        override protected void OnCreate()
        {
            Logger.Debug("");

            base.OnCreate();

            // NOTE To use theme.xaml, uncomment below line.
            // ThemeManager.ApplyTheme(new Theme(Tizen.Applications.Application.Current.DirectoryInfo.Resource + "theme/theme.xaml"));
            mainPage = new MainPage();

            GetDefaultWindow().Add(mainPage);
            GetDefaultWindow().KeyEvent += OnKeyEvent;
        }

        private void OnKeyEvent(object sender, Window.KeyEventArgs e)
        {
            Logger.Debug("");

            if (e.Key.State == Key.StateType.Down && (e.Key.KeyPressedName == "XF86Back" || e.Key.KeyPressedName == "Escape"))
            {
                Exit();
            }
        }

        override protected void OnPause()
        {
            base.OnPause();
        }

        override protected void OnResume()
        {
            base.OnResume();
        }

        override protected void OnTerminate()
        {
            Logger.Debug("");

            mainPage.DataUpdateStop();
            base.OnTerminate();
        }

        override protected void OnAppControlReceived(AppControlReceivedEventArgs e)
        {
            base.OnAppControlReceived(e);
        }
    }
}

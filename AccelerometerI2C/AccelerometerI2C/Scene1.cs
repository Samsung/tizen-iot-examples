using Tizen.Applications;
using Tizen.NUI;

namespace AccelerometerI2C
{
    public class Scene1 : NUIApplication
    {
        private Scene1Page scene1Page;
        override protected void OnCreate()
        {
            Logger.Debug("");

            base.OnCreate();

            // NOTE To use theme.xaml, uncomment below line.
            // ThemeManager.ApplyTheme(new Theme(Tizen.Applications.Application.Current.DirectoryInfo.Resource + "theme/theme.xaml"));
            scene1Page = new Scene1Page();

            GetDefaultWindow().Add(scene1Page);
            GetDefaultWindow().KeyEvent += OnScene1KeyEvent;
        }

        private void OnScene1KeyEvent(object sender, Window.KeyEventArgs e)
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

            scene1Page.DataUpdateStop();
            base.OnTerminate();
        }

        override protected void OnAppControlReceived(AppControlReceivedEventArgs e)
        {
            base.OnAppControlReceived(e);
        }
    }
}

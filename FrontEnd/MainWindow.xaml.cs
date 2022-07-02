using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace FrontEnd
{
	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class MainWindow : Window
	{
		bool is_webview_init = false;

		public MainWindow()
		{
			InitializeComponent();
		}

		async void windowSizeChanged(object sender, SizeChangedEventArgs e)
		{
			webview.Width = e.NewSize.Width;
			webview.Height = e.NewSize.Height;

			if (is_webview_init == false) {

				// it ain't doing jack shit, still crash when call from separate thread to early
				await webview.EnsureCoreWebView2Async();
	#if DEBUG
				DevGlobals.init(this, webview);
				DevGlobals.initHtmlCssHotReload();
				DevGlobals.initTypeScriptHotReload();
	#endif
				webViewInit();

				is_webview_init = true;
			}
		}

		private void webViewInit()
		{		
			webview.CoreWebView2.SetVirtualHostNameToFolderMapping(
				"app.invalid", "./ClientSide/", Microsoft.Web.WebView2.Core.CoreWebView2HostResourceAccessKind.Allow
			);
			// webview.CoreWebView2.Settings

			// source.CoreWebView2.AddWebResourceRequestedFilter();
			// source.CoreWebView2.WebResourceRequested

			webview.Source = new Uri("http://app.invalid/index.html");
		}
	}
}

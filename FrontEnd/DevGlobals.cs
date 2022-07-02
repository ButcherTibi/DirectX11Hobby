using System;
using System.IO;
using System.Text.Json;
using System.Text.Json.Nodes;
using System.Threading;
using System.Collections.Generic;
using WebView2 = Microsoft.Web.WebView2.Wpf.WebView2;


public static class DevGlobals
{
	static string project_dir;
	static string output_dir;
		  
	static string client_side_src;
	static string client_side_dest;

	static FrontEnd.MainWindow main_window;
	static WebView2 main_webview;

	static Thread hot_reload_thread;
	static Dictionary<string, FileCopyStatus> copied_files;


	static DevGlobals() {
		output_dir = AppDomain.CurrentDomain.BaseDirectory;

		project_dir = Path.GetFullPath(Path.Combine(
			AppDomain.CurrentDomain.BaseDirectory,
			@"..\..\..\"
		));

		string client_dir_name = "ClientSide";
		client_side_src = Path.Join(project_dir, client_dir_name);
		client_side_dest = Path.Join(output_dir, client_dir_name);

		copied_files = new Dictionary<string, FileCopyStatus>();
	}

	public static void init(FrontEnd.MainWindow new_window, WebView2 new_webview)
	{
		main_window = new_window;
		main_webview = new_webview;
	}

	//public static bool reloadWebView()
	//{
	//	return main_window.DispatcherQueue.TryEnqueue(() => {
	//		main_webview.Reload();
	//	});
	//}

	//public static void configureTypeScriptOutDir()
	//{
	//	string ts_config_path = Path.Join(client_side_src, "tsconfig.json");
	//	string ts_config = File.ReadAllText(ts_config_path);

	//	var parse_settings = new JsonDocumentOptions();
	//	parse_settings.CommentHandling = JsonCommentHandling.Skip;
	//	JsonNode json = JsonNode.Parse(ts_config, documentOptions: parse_settings);

	//	json["compilerOptions"]["outDir"] = client_side_dest;

	//	File.WriteAllText(ts_config_path, json.ToJsonString());
	//}

	public static void initTypeScriptHotReload()
	{
		string input_files = "";
		{
			void addInputFiles(string dir_path)
			{
				foreach (var file_path in Directory.GetFiles(dir_path)) {

					if (Path.GetExtension(file_path) == ".ts") {

						string relative_path = file_path.Split(client_side_src + "\\")[1];
						input_files += $"\"{relative_path}\" ";
					}
				}

				foreach (var child_dir in Directory.GetDirectories(dir_path)) {
					addInputFiles(child_dir);
				}
			}

			addInputFiles(client_side_src);
		}

		var process = new System.Diagnostics.Process();
		process.StartInfo.FileName = "CMD.exe";
		process.StartInfo.Arguments =$"/c tsc " + input_files +
			"--target es2021 " +
			"--module es2020 " +
			$"--outDir \"{client_side_dest}\" " +
			"--strict true " +
			"--sourceMap true " +
			"--forceConsistentCasingInFileNames true " +
			"--skipLibCheck true " +
			"--watch";
		process.StartInfo.CreateNoWindow = false;
		process.StartInfo.WorkingDirectory = client_side_src;
		//process.StartInfo.RedirectStandardOutput = true;
		//process.OutputDataReceived += (sender, e) =>
		//{
		//	if (e.Data != null || e.Data != "") {
		//		System.Diagnostics.Debug.WriteLine(e.Data);
		//	}
		//};

		process.Start();
		// process.BeginOutputReadLine();

		// process.WaitForExit();
	}

	class FileCopyStatus
	{
		public DateTime last_copied;
	}

	public static void initHtmlCssHotReload()
	{
		hot_reload_thread = new Thread(() =>
		{
			while (true) {

				bool were_files_copied = false;

				void addInputFiles(string dir_path)
				{
					foreach (var src_filepath in Directory.GetFiles(dir_path)) {

						string relative_path = src_filepath.Split(client_side_src + "\\")[1];
						string dest_filepath = Path.Join(client_side_dest, relative_path);

						switch (Path.GetExtension(src_filepath)) {
						case ".html":
						case ".css": {

							FileCopyStatus status;
							if (copied_files.TryGetValue(src_filepath, out status)) {

								// file was updated
								var last_write_time = File.GetLastWriteTimeUtc(src_filepath);
								if (status.last_copied < last_write_time) {

									status.last_copied = last_write_time;
									File.Copy(src_filepath, dest_filepath, true);
									were_files_copied = true;
								}
							}
							// file never copied over
							else {
								copied_files.Add(src_filepath, new FileCopyStatus {
									last_copied = DateTime.UtcNow
								});

								File.Copy(src_filepath, dest_filepath, true);
								were_files_copied = true;
							}
							break;
						}
						}
					}

					foreach (var child_dir in Directory.GetDirectories(dir_path)) {
						addInputFiles(child_dir);
					}
				}

				addInputFiles(client_side_src);

				if (were_files_copied) {
					// reloadWebView();
				}

				Thread.Sleep(1000 * 2);
			}
		});

		hot_reload_thread.Start();
	}
}
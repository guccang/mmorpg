using UnityEngine;
using System.Collections;
using System.IO;
using System.Text;

public class localIO {
	static FileStream fs;
	static StreamWriter sw;
	public static void Init(string fileName)
	{
		if(!Directory.Exists("./Dump"))
			Directory.CreateDirectory("./Dump");
		fs = new FileStream("./Dump/"+fileName, FileMode.Create);
		 sw = new StreamWriter(fs);
	}

	public static void Write(string data)
	{
		sw.Write(data);
		sw.Flush();
	}

	public static void close()
	{
		sw.Close();
		fs.Close();
	}
}

using System;
using System.IO;

namespace FSCC
{
	class MainClass
	{
		public static void Main(string[] args)
		{
		    FSCC.Port port = null;
		    
		    try
		    {
			    port = new Port("/dev/fscc0", FileAccess.Read);
			}
			catch (System.UnauthorizedAccessException e)
			{
			    Console.WriteLine(e.Message);
			    
			    return;
			}
			
			Console.WriteLine(port.Read(4096));
			
			port.Close();
		}
	}
}

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
			    port = new Port("/dev/fscc0", FileAccess.Write);
			}
			catch (System.UnauthorizedAccessException e)
			{
			    Console.WriteLine(e.Message);
			    
			    return;
			}
					
			port.Write("Hello world!");
			
			port.Close();
		}
	}
}

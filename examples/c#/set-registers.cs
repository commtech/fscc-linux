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

		    port.Registers.CCR0 = 0x0011201c;
		    port.Registers.CCR1 = 0x00000018;
		    port.Registers.CCR2 = 0x00000000;

			port.Close();
		}
	}
}

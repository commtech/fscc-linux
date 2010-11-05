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

            Console.WriteLine(String.Format("CCR0 = 0x{0:x8}", port.Registers.CCR0));
            Console.WriteLine(String.Format("CCR1 = 0x{0:x8}", port.Registers.CCR1));
            Console.WriteLine(String.Format("CCR2 = 0x{0:x8}", port.Registers.CCR2));

			port.Close();
		}
	}
}

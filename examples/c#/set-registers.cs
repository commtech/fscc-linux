/*
    Copyright (C) 2013  Commtech, Inc.

    This file is part of fscc-linux.

    fscc-linux is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    fscc-linux is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with fscc-linux.  If not, see <http://www.gnu.org/licenses/>.

*/

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

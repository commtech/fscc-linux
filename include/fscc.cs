/*
	Copyright (C) 2011 Commtech, Inc.

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
using System.Runtime.InteropServices;
using System.Text;

namespace FSCC
{
	public class Registers
	{
	    public Registers()
	    {
	    }

		public int FIFOT { get; set; }
		public int STAR { get; set; }
		public int CCR0 { get; set; }
		public int CCR1 { get; set; }
		public int CCR2 { get; set; }
		public int BGR { get; set; }
		public int SSR { get; set; }
		public int TSR { get; set; }
		public int TMR { get; set; }
		public int RAR { get; set; }
		public int RAMR { get; set; }
		public int PPR { get; set; }
		public int TCR { get; set; }
		public int VSTR { get; set; }
		public int IMR { get; set; }
		public int DPLLR { get; set; }
		public int FCR { get; set; }
	}

	public class Port : System.IO.FileStream
	{
		public const int FLUSH_TX = 6146;
		public Registers Registers;

		public Port(string path, FileAccess access) : base(path, FileMode.Open, access)
		{
			this.fd = this.Open(path, 1);
			this.Registers = new Registers();
		}

		~Port()
		{
		    this.Close(this.fd);
		}

		public void Write(string data)
		{
			System.Text.ASCIIEncoding encoder = new System.Text.ASCIIEncoding();
			byte[] output_bytes = encoder.GetBytes(data);

			this.Write(output_bytes, 0, output_bytes.Length);
		}

		public string Read(int count)
		{
			System.Text.ASCIIEncoding encoder = new System.Text.ASCIIEncoding();
			byte[] input_bytes = new byte[count];

			this.Read(input_bytes, 0, count);

			return encoder.GetString(input_bytes);
		}

		public bool AppendStatus
		{
		    set
			{
					throw new System.NotImplementedException();

					/*
					if (value == true)
						this.Ioctl(FSCC_ENABLE_APPEND_STATUS)
					else
						tihs.Ioctl(FSCC_DISABLE_APPEND_STATUS)
					*/
			}
		}

		/* Cannot pass in File. Must use FileStream or something else
		public void ImportRegisters(File import_file)
		{
			throw new System.NotImplementedException();
		}

		public void ExportRegisters(File export_file)
		{
			throw new System.NotImplementedException();
		}
		*/

		public void FlushRx()
		{
			throw new System.NotImplementedException();

			/*
			this.Ioctl(Port.FLUSH_RX);
			*/
		}

		public void FlushTx()
		{
			throw new System.NotImplementedException();

			/*
			this.Ioctl(Port.FLUSH_TX);
			*/
		}

		public int Ioctl(int request)
		{
			return this.IoctlWorker(this.fd, request);
		}

		[DllImport("libc.so.6", EntryPoint="open")]
		private static extern int Open(string path, int oflag);

		[DllImport("libc.so.6", EntryPoint="close")]
		private static extern int Close(int fd);

		[DllImport("libc.so.6", EntryPoint="ioctl")]
		private static extern int IoctlWorker(int fd, int request);
		private int fd;
	}
}

/*
	Copyright (C) 2010  Commtech, Inc.

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
	public class Port : System.IO.FileStream
	{
		public Port(string path, FileAccess access) : base(path, FileMode.Open, access)
		{
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
	}
}

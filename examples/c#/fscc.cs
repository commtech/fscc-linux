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

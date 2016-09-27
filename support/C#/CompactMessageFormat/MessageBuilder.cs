using System;
using System.Diagnostics;
using System.Text;

namespace CMF
{
	public class MessageBuilder
	{
		public MessageBuilder(byte[] data, int position) {
			buffer = data;
			this.position = position;
		}

		public void Add(int tag, long value) {
			ValueType vt;
			if (value >= 0){
				vt = ValueType.POSITIVE_NUMBER;
			} else {
				vt = ValueType.NEGATIVE_NUMBER;
				value *= -1;
			}
			write(tag, vt);
			VarInt vi = new VarInt(buffer, position);
			position += vi.Serialize(value);
		}

		public void Add(int tag, int value) {
			Add(tag, (long) value);
		}

		public void Add(int tag, String value) {
			write(tag, ValueType.STRING);
			byte[] serializedData = Encoding.UTF8.GetBytes(value);
			VarInt vi = new VarInt(buffer, position);
			position += vi.Serialize(serializedData.Length);
			Buffer.BlockCopy(serializedData, 0, buffer, position, serializedData.Length);
			position += serializedData.Length;
		}

		public void Add(int tag, byte[] value) {
			write(tag, ValueType.BYTEARRAY);
			VarInt vi = new VarInt(buffer, position);
			position += vi.Serialize(value.Length);
			Buffer.BlockCopy(value, 0, buffer, position, value.Length);
			position += value.Length;
		}

		public void Add(int tag, bool value) {
			write(tag, value ? ValueType.BOOL_TRUE : ValueType.BOOL_FALSE);
		}

		public int GetPosition() {
			return position;
		}

		private byte[] buffer = null;
		private int position = -1;

		private void write(int tag, ValueType type) {
			if (tag >= 31) { // use more than 1 byte
				byte b = (byte)((byte) type | 0xF8); // set the 'tag' to all 1s
				buffer [position] = b;
				VarInt vi = new VarInt (buffer, position + 1);
				position += vi.Serialize (tag) + 1;
				return;
			} else {
				Debug.Assert(tag < 32);
				byte b = (byte)tag;
				b = (byte)(b << 3);
				b += (byte) type;
				buffer [position++] = b;
			}
		}
	}
}


using System;

namespace CMF
{
	public class VarInt
	{
		public VarInt(byte[] data, int positionInStream) {
			this.data = data;
			position = positionInStream;
		}

		/// After doing a serialize or deserialize the position will have updated.
		public int GetPosition() {
			return position;
		}

		public long Unserialize() {
			long result = 0;
			int pos = position;
			while (pos - position < 8 && pos < data.Length) {
				byte b = data[pos++];
				result = (result << 7) | (byte)(b & 0x7F);
				if ((b & 0x80) != 0)
					result++;
				else {
					position = pos;
					return result;
				}
			}
			throw new Exception("Reading VarInt past stream-size");
		}

		/**
	     * Append to the data block the serialized version of value, returning the amount of bytes used.
    	 */
		public int Serialize(long value) {
			int pos = 0;
			while (true) {
				// data[position + pos] = ((byte) (value & 0x7F)) | ((byte) (pos != 0 ? 0x80 : 0x00));
				byte step1 = (byte)(value & 0x7F);
				byte step2 = (byte) ((pos != 0) ? 0x80 : 0x00);
				data[position + pos] = (byte) (step1 | step2);
				if (value <= 0x7F)
					break;
				value = (value >> 7) - 1;
				pos++;
			}

			// reverse
			for (int i = pos / 2; i >= 0; --i) {
				// swap them
				byte tmp = data[position + i];
				data[position + i] = data[position + pos - i];
				data[position + pos - i] = tmp;
			}
			position += ++pos;
			return pos;
		}

		private byte[] data = null;
		private int position = -1;
	}
}

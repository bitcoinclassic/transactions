/* Copyright (c) 2016 Tom Zander <tomz@freedommail.ch>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
using System;
using System.Diagnostics;
using System.Text;

namespace CMF
{
	/**
	 * Message builder creates a generic message in the form of
	 *    name: value
	 * pairs, in a flat list.
	 *
	 * The useful part is that the value is typed and for variable-length structures
	 * (like a string) a length is included.
	 * The effect is that you can fully parse a structure without having any prior knowledge
	 * of the fields, the expected content in the fields and the size.
	 * You can compare this to an XML stream where some items are stored with tags or attributes
	 * are unknown to the reader, without causing any effect on being able to parse them or to write
	 * them out again unchanged.
	 */
	public class MessageBuilder
	{
		public MessageBuilder(byte[] data, int position) {
			buffer = data;
			this.position = position;
		}

		public void Add(int tag, long value) {
			ValueType vt;
			if (value >= 0){
				vt = ValueType.PositiveNumber;
			} else {
				vt = ValueType.NegativeNumber;
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
			write(tag, ValueType.String);
			byte[] serializedData = Encoding.UTF8.GetBytes(value);
			VarInt vi = new VarInt(buffer, position);
			position += vi.Serialize(serializedData.Length);
			Buffer.BlockCopy(serializedData, 0, buffer, position, serializedData.Length);
			position += serializedData.Length;
		}

		public void Add(int tag, byte[] value) {
			write(tag, ValueType.ByteArray);
			VarInt vi = new VarInt(buffer, position);
			position += vi.Serialize(value.Length);
			Buffer.BlockCopy(value, 0, buffer, position, value.Length);
			position += value.Length;
		}

		public void Add(int tag, bool value) {
			write(tag, value ? ValueType.BoolTrue : ValueType.BoolFalse);
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


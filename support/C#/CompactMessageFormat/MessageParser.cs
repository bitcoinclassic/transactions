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
	 * MessageParser takes a CMF stream and parses it into tokens.
	 *
	 * A user can iterate over tokens by repeatedly calling next() and as
	 * long as FoundTag is returned you can then find the token-tag
	 * as well as the actual data. The MessageParser will actually have typed
	 * data based on what was encoded in the CMF stream. You can access
	 * this using the different getters like GetLong() GetString(). Please note
	 * that if the requested data is not what was present in the stream you will
	 * end up throwing an exception on those getters.
	 */
	public class MessageParser
	{
		public MessageParser(byte[] data, int position, int length) {
			this.data = data;
			this.position = position;
			endPosition = position + length;
			Debug.Assert(data.Length >= endPosition);
		}

		public enum State {
			FoundTag,
			EndOfDocument,
			Error
		};

		public State Next() {
			if (endPosition <= position)
				return State.EndOfDocument;

			int b = data[position];
			ValueType type = (ValueType)(b & 7);
			tag = b >> 3;
			if (tag == 31) { // the tag is stored in the next byte(s)
				try {
					VarInt vi = new VarInt(data, ++position);
					long newTag = vi.Unserialize();
					if (newTag > 0xFFFF) {
						Console.WriteLine("Malformed tag-type "+ newTag + " is a too large enum value");
						return State.Error;
					}
					position = vi.GetPosition() - 1;
					tag = (int) newTag;
				} catch (Exception e) {
					Console.WriteLine("Malformed varint; "+ e.Message);
					return State.Error;
				}
			}

			switch (type) {
			case ValueType.NegativeNumber:
			case ValueType.PositiveNumber:
				try {
					VarInt vi = new VarInt(data, ++position);
					long value = vi.Unserialize();
					position = vi.GetPosition();
					if (type == ValueType.NegativeNumber)
						this.value = value * -1;
					else
						this.value = value;
				} catch (Exception e) {
					Console.WriteLine("Malformed varint; "+ e.Message);
					return State.Error;
				}
				break;
			case ValueType.ByteArray:
			case ValueType.String:
				try {
					VarInt vi = new VarInt(data, ++position);
					int length = (int) vi.Unserialize();
					if (vi.GetPosition() + length > data.Length) // need more bytes
						return State.EndOfDocument;

					valueState = type == ValueType.ByteArray ? Lazy.ByteArray : Lazy.String;
					dataStart = vi.GetPosition();
					dataLength = length;
					position = dataStart + length;
				} catch (Exception e) {
					Console.WriteLine("Malformed varint; "+ e.Message);
					return State.Error;
				}
				break;
			case ValueType.BoolTrue:
				value = true;
				++position;
				break;
			case ValueType.BoolFalse:
				value = false;
				++position;
				break;
			default:
				return State.Error;
			}
			return State.FoundTag;
		}

		public int GetTag() {
			return tag;
		}

		public int Consumed() {
			return position;
		}

		public void Consume(int bytes) {
			Debug.Assert(bytes >= 0);
			position =+ bytes;
		}

		public int GetInt() {
			Int64 answer = (Int64) value;
			if (answer > 0x7FFFFFFF)
				throw new Exception("Too big");
			return (int) answer;
		}

		public ulong GetLong() {
			return (UInt64) value;
		}

		public bool GetBoolean() {
			return (Boolean) value;
		}

		public String GetString() {
			if (valueState == Lazy.String) {
				byte[] raw = new byte[dataLength];
				Buffer.BlockCopy(data, dataStart, raw, 0, dataLength);
				String answer = Encoding.UTF8.GetString(raw);
				value = answer;
				valueState = Lazy.Parsed;
				return answer;
			}
			return (String) value;
		}

		public byte[] getByteArray() {
			if (valueState == Lazy.ByteArray) {
				byte[] answer = new byte[dataLength];
				Buffer.BlockCopy(data, dataStart, answer, 0, dataLength);
				value = answer;
				return answer;
			}
			return new byte[0];
		}

		private byte[] data = null;
		private int position = 0;
		private int endPosition = 0;
		private int tag = 0;

		private enum Lazy {
			Parsed,
			ByteArray,
			String
		};
		private Lazy valueState = Lazy.Parsed;
		private int dataStart = -1;
		private int dataLength = -1;
		private Object value = null;
	}
}

using System;
using System.Diagnostics;
using System.Text;

namespace CMF
{
	public class MessageParser
	{
		public MessageParser(byte[] data, int position, int length) {
			this.data = data;
			this.position = position;
			endPosition = position + length;
			Debug.Assert(data.Length >= endPosition);
		}

		public enum State {
			FOUND_TAG,
			END_OF_DOCUMENT,
			ERROR
		};

		public State Next() {
			if (endPosition <= position)
				return State.END_OF_DOCUMENT;

			int b = data[position];
			ValueType type = (ValueType)(b & 7);
			tag = b >> 3;
			if (tag == 31) { // the tag is stored in the next byte(s)
				try {
					VarInt vi = new VarInt(data, ++position);
					long newTag = vi.Unserialize();
					if (newTag > 0xFFFF) {
						Console.WriteLine("Malformed tag-type "+ newTag + " is a too large enum value");
						return State.END_OF_DOCUMENT;
					}
					position = vi.GetPosition() - 1;
					tag = (int) newTag;
				} catch (Exception) {
					return State.END_OF_DOCUMENT;
				}
			}

			switch (type) {
			case ValueType.NEGATIVE_NUMBER:
			case ValueType.POSITIVE_NUMBER:
				try {
					VarInt vi = new VarInt(data, ++position);
					long value = vi.Unserialize();
					position = vi.GetPosition();
					if (type == ValueType.NEGATIVE_NUMBER)
						this.value = value * -1;
					else
						this.value = value;
				} catch (Exception) {
					return State.END_OF_DOCUMENT;
				}
				break;
			case ValueType.BYTEARRAY:
			case ValueType.STRING:
				try {
					VarInt vi = new VarInt(data, ++position);
					int length = (int) vi.Unserialize();
					if (vi.GetPosition() + length > data.Length) // need more bytes
						return State.END_OF_DOCUMENT;

					valueState = type == ValueType.BYTEARRAY ? Lazy.ByteArray : Lazy.String;
					dataStart = vi.GetPosition();
					dataLength = length;
					position = dataStart + length;
				} catch (Exception) {
					return State.END_OF_DOCUMENT;
				}
				break;
			case ValueType.BOOL_TRUE:
				value = true;
				++position;
				break;
			case ValueType.BOOL_FALSE:
				value = false;
				++position;
				break;
			default:
				return State.ERROR;
			}
			return State.FOUND_TAG;
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

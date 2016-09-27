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
using NUnit.Framework;
using System;
using System.Text;

namespace CMF.test
{
	[TestFixture ()]
	public class NUnitTestClass
	{
		[Test ()]
		public void TestCase ()
		{
			byte[] buffer = new byte[100];
			{
				MessageBuilder builder = new MessageBuilder(buffer, 0);
				builder.Add(15, 6512);
				Assert.AreEqual(3, builder.GetPosition());
			}
			Assert.AreEqual(120, buffer[0]);
			Assert.AreEqual((byte) 177, buffer[1]);
			Assert.AreEqual(112, buffer[2]);

			{
				MessageParser parser = new MessageParser(buffer, 0, 3);
				Assert.AreEqual(MessageParser.State.FoundTag, parser.Next());
				Assert.AreEqual(15, parser.GetTag());
				Assert.AreEqual(6512, parser.GetInt());
				Assert.AreEqual(MessageParser.State.EndOfDocument, parser.Next());
			}

			{
				MessageBuilder builder = new MessageBuilder(buffer, 0);
				builder.Add(129, 6512);
				Assert.AreEqual(5, builder.GetPosition());
			}
			Assert.AreEqual((byte) 248, buffer[0]);
			Assert.AreEqual((byte) 128, buffer[1]);
			Assert.AreEqual(1, buffer[2]);
			Assert.AreEqual((byte) 177, buffer[3]);
			Assert.AreEqual((byte) 112, buffer[4]);

			{
				MessageParser parser = new MessageParser(buffer, 0, 5);
				Assert.AreEqual(MessageParser.State.FoundTag, parser.Next());
				Assert.AreEqual(129, parser.GetTag());
				Assert.AreEqual(6512, parser.GetInt());
				Assert.AreEqual(MessageParser.State.EndOfDocument, parser.Next());
			}
		}


		[Test ()]
		public void TestTypes()
		{
			byte[] buffer = new byte[100];
			MessageBuilder builder = new MessageBuilder(buffer, 0);
			builder.Add(1, "Föo");
			byte[] blob = Encoding.UTF8.GetBytes("hihi");
			builder.Add(200, blob);
			builder.Add(3, true);
			builder.Add(40, false);

			Assert.AreEqual(17, builder.GetPosition());

			// string '1'
			Assert.AreEqual((byte) 10, buffer[0]);
			Assert.AreEqual((byte) 4, buffer[1]); // serialized string length
			Assert.AreEqual((byte) 70, buffer[2]);
			Assert.AreEqual((byte) 195, buffer[3]);
			Assert.AreEqual((byte) 182, buffer[4]);
			Assert.AreEqual((byte) 111, buffer[5]);

			// blob '200'
			Assert.AreEqual((byte) 251, buffer[6]);
			Assert.AreEqual((byte) 128, buffer[7]);
			Assert.AreEqual((byte) 72, buffer[8]);
			Assert.AreEqual((byte) 4, buffer[9]); // length of bytearray
			Assert.AreEqual((byte) 104, buffer[10]); // 'h'
			Assert.AreEqual((byte) 105, buffer[11]); // 'i'
			Assert.AreEqual((byte) 104, buffer[12]); // 'h'
			Assert.AreEqual((byte) 105, buffer[13]); // 'i'

			// bool-true '3'
			Assert.AreEqual((byte) 28, buffer[14]);

			// bool-false '40'
			Assert.AreEqual((byte) 253, buffer[15]);
			Assert.AreEqual((byte) 40, buffer[16]);

			MessageParser parser = new MessageParser(buffer, 0, builder.GetPosition());
			Assert.AreEqual(MessageParser.State.FoundTag, parser.Next());
			Assert.AreEqual(1, parser.GetTag());
			Assert.AreEqual("Föo", parser.GetString());
			Assert.AreEqual(MessageParser.State.FoundTag, parser.Next());
			Assert.AreEqual(200, parser.GetTag());
			byte[] parsed = parser.getByteArray();
			Assert.AreEqual(blob.Length, parsed.Length);
			for (int i = 0; i < blob.Length; ++i) {
				Assert.AreEqual(blob[i], parsed[i]);
			}
			Assert.AreEqual(MessageParser.State.FoundTag, parser.Next());
			Assert.AreEqual(3, parser.GetTag());
			Assert.AreEqual(true, parser.GetBoolean());
			Assert.AreEqual(MessageParser.State.FoundTag, parser.Next());
			Assert.AreEqual(40, parser.GetTag());
			Assert.AreEqual(false, parser.GetBoolean());
			Assert.AreEqual(MessageParser.State.EndOfDocument, parser.Next());
		}
	}
}

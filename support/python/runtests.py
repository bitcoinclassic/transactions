#!/usr/bin/python2
# -*- coding: UTF-8 -*-
# Copyright (c) 2016-2017 Tom Zander <tomz@freedommail.ch>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from compactmessageformat import *
import unittest


class TestCMF(unittest.TestCase):

    def test_basic(self):
        buffer = bytearray(b"000000000000000000000000000000000")
        builder = MessageBuilder(buffer, 0)
        builder.add_int(15, 6512)
        self.assertEqual(builder.get_position(), 3)
        self.assertEqual(buffer[0], 120)
        self.assertEqual(buffer[1], 177)
        self.assertEqual(buffer[2], 112)

        parser = MessageParser(buffer, 0, 3);
        self.assertEquals(MessageParser.Type.FoundTag, parser.next());
        self.assertEquals(15, parser.tag);
        self.assertEquals(6512, parser.value);
        self.assertEquals(MessageParser.Type.EndOfDocument, parser.next());

    def test_basic2(self):
        buffer = bytearray(b"000000000000000000000000000000000")
        builder = MessageBuilder(buffer, 0)
        builder.add_int(129, 6512)
        self.assertEqual(builder.get_position(), 5)
        self.assertEqual(buffer[0], 248)
        self.assertEqual(buffer[1], 128)
        self.assertEqual(buffer[2], 1)
        self.assertEqual(buffer[3], 177)
        self.assertEqual(buffer[4], 112)

        parser = MessageParser(buffer, 0, 5);
        self.assertEquals(MessageParser.Type.FoundTag, parser.next());
        self.assertEquals(129, parser.tag);
        self.assertEquals(6512, parser.value);
        self.assertEquals(MessageParser.Type.EndOfDocument, parser.next());

    def test_types(self):
        buffer = bytearray(b"000000000000000000000000000000000")
        builder = MessageBuilder(buffer, 0)
        builder.add_string(1, "Föo");
        builder.add_bytes(200, b"hihi");
        builder.add_bool(3, True);
        builder.add_bool(40, False);
        self.assertEqual(builder.get_position(), 17)

        # string '1'
        self.assertEquals(buffer[0], 10)
        self.assertEquals(buffer[1], 4) # serialized string length
        self.assertEquals(buffer[2], 70)
        self.assertEquals(buffer[3], 195)
        self.assertEquals(buffer[4], 182)
        self.assertEquals(buffer[5], 111)

        # blob '200'
        self.assertEquals(buffer[6], 251)
        self.assertEquals(buffer[7], 128)
        self.assertEquals(buffer[8], 72)
        self.assertEquals(buffer[9], 4) #length of bytearray
        self.assertEquals(buffer[10], 104) # 'h'
        self.assertEquals(buffer[11], 105) # 'i'
        self.assertEquals(buffer[12], 104) # 'h'
        self.assertEquals(buffer[13], 105) # 'i'

        # bool-true '3'
        self.assertEquals(buffer[14], 28);

        # bool-false '40'
        self.assertEquals(buffer[15], 253);
        self.assertEquals(buffer[16], 40);

        parser = MessageParser(buffer, 0, builder.get_position());
        self.assertEquals(MessageParser.Type.FoundTag, parser.next());
        self.assertEquals(1, parser.tag);
        self.assertEquals(bytes("Föo"), parser.string_value())
        self.assertEquals(MessageParser.Type.FoundTag, parser.next());
        self.assertEquals(200, parser.tag);
        self.assertEquals(b"hihi", parser.string_value())
        self.assertEquals(MessageParser.Type.FoundTag, parser.next());
        self.assertEquals(3, parser.tag);
        self.assertEquals(True, parser.value)
        self.assertEquals(MessageParser.Type.FoundTag, parser.next());
        self.assertEquals(40, parser.tag);
        self.assertEquals(False, parser.value)
        self.assertEquals(MessageParser.Type.EndOfDocument, parser.next());

if __name__ == '__main__':
    unittest.main()


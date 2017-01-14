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

import codecs

class CMF_ValueType:
    PositiveNumber = 0, # var-int-encoded (between 1 and 9 bytes in length). Per definition a positive number.
    NegativeNumber = 1, # var-int-encoded (between 1 and 9 bytes in length). Per definition a negative number.
    String = 2,         # first an UnsignedNumber for the length, then the actual bytes. Never a closing zero. Utf8 encoded.
    ByteArray = 3,      # identical to String, but without encoding.
    BoolTrue = 4,       # not followed with any bytes
    BoolFalse = 5       # not followed with any bytes

def serialize(data, offset, value):
    pos = 0
    while (1):
        mask = 0
        if (pos != 0):
            mask = 0x80

        data[pos + offset] = (value & 0x7F) | mask
        if (value <= 0x7F):
            break
        value = (value >> 7) - 1
        pos = pos + 1

    # reverse
    i = int(pos / 2)
    while (i >= 0):
        byte = data[pos + offset - i]
        data[pos + offset - i] = data[i + offset]
        data[i + offset] = byte
        i = i - 1

    return pos + 1


def unserialize(data, dataSize, position):
    assert(position >= 0)
    result = 0
    pos = position
    while (pos - position < 8):
        byte = data[pos]
        pos += 1
        result = (result << 7) | (byte & 0x7F)
        if ((byte & 0x80) != 0):
            result += 1
        else:
            position = pos
            return position, result
    raise Exception("Reading VarInt past stream-size")

def arraycopy(source, sourcePos, dest, destPos, numElem):
    while (numElem > 0):
        dest[destPos] = source[sourcePos]
        numElem -= 1
        destPos += 1
        sourcePos += 1

"""
  Message builder creates a generic message in the form of
     name: value
  pairs, in a flat list.

  The unique part is that the value is typed and for variable-length structures
  (like a string) a length is included.
  The effect is that you can fully parse a structure without having any prior knowledge
  of the fields, the expected content in the fields and the size.
  You can compare this to an XML stream where some items are stored with tags or attributes
  are unknown to the reader, without causing any effect on being able to parse them or to write
  them out again unchanged.
"""
class MessageBuilder:
    def __init__(self, data, position):
        self.buffer = data
        self.position = position

    def add_int(self, tag, value):
        if (value >= 0):
            vt = 0 # PositiveNumber
        else:
            vt = 1 # NegativeNumber
            value *= -1
        self.__write(tag, vt)
        self.position += serialize(self.buffer, self.position, value)

    # This method assumes that 'value' is already an utf8 encoded string.
    def add_string(self, tag, value):
        self.__write(tag, 2) # String
        self.position += serialize(self.buffer, self.position, len(value))
        arraycopy(value, 0, self.buffer, self.position, len(value))
        self.position += len(value)

    def add_bytes(self, tag, value):
        self.__write(tag, 3) # bytearray
        self.position += serialize(self.buffer, self.position, len(value))
        arraycopy(value, 0, self.buffer, self.position, len(value))
        self.position += len(value)

    def add_bool(self, tag, value):
        type = 5 # Bool_False
        if (value == True):
            type = 4 # Bool_True
        self.__write(tag, type)

    def get_position(self):
        return self.position

    def __write(self, tag, type):
        if (tag >= 31): # use more than 1 byte
            byte = type | 0xF8 # set the 'tag' to all 1s
            self.buffer[self.position] = byte
            self.position += serialize(self.buffer, self.position + 1, tag) + 1
            return
        assert(tag < 32)
        byte = tag
        byte = byte << 3
        byte += type
        self.buffer[self.position] = byte
        self.position = self.position + 1

class MessageParser(object):
    def __init__(self, data, position, length):
        self.data = data
        self.position = position
        self.endPosition = position + length
        self.tag = -1
        self.valueState = MessageParser.Lazy.ValueParsed
        self.dataStart = -1
        self.dataLength = -1

    class Type:
        FoundTag = 0
        EndOfDocument = 1
        Error = 3

    class Lazy:
        ValueParsed = 0
        ByteArray = 1
        String = 1

    def next(self):
        if (self.endPosition <= self.position):
            return MessageParser.Type.EndOfDocument

        byte = self.data[self.position]
        data_type = (byte & 0x07)

        self.tag = byte >> 3
        if (self.tag == 31):  # the tag is stored in the next byte(s)
            newTag = 0
            self.position += 1
            self.position, newTag = unserialize(self.data, self.endPosition, self.position)
            ok = True
            if (ok and newTag > 0xFFFF):
                ok = False
            self.position -= 1
            if (not ok):
                return MessageParser.Type.Error
            self.tag = newTag

        value = 0
        if (data_type == 0 or data_type == 1): # Numbers
            self.position, value = unserialize(self.data, self.endPosition, self.position + 1)
            if (data_type == 1): # CMF_ValueType.NegativeNumber
                value *= -1
            self.value = value
        elif (data_type == 2 or data_type == 3): # String or ByteArray
            newPos = self.position + 1
            newPos, value = unserialize(self.data, self.endPosition, newPos)
            if (newPos + value > len(self.data)): # need more bytes
                return MessageParser.Type.Error

            self.valueState = MessageParser.Lazy.ByteArray if (data_type == 3) else MessageParser.Lazy.String
            self.dataStart = newPos
            self.dataLength = value
            self.position = newPos + value
        elif (data_type == 4):
            value = True
            self.position += 1
        elif (data_type == 5):
            value = False
            self.position += 1
        else:
            return MessageParser.Type.Error

        self.value = value
        return MessageParser.Type.FoundTag

    def string_value(self):
        if (self.valueState == MessageParser.Lazy.ByteArray or self.valueState == MessageParser.Lazy.String):
            return self.data[self.dataStart:self.dataStart + self.dataLength]
        return self.value

    def consumed(self):
        return self.position

    def consume(self, num_bytes):
        assert(num_bytes >= 0)
        self.position += num_bytes



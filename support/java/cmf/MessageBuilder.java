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
package cmf;


/**
 * Message builder creates a generic message in the form of
 *    name: value
 * pairs, in a flat list.
 *
 * The unique part is that the value is typed and for variable-length structures
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

    public void add(int tag, long value) {
        ValueType vt;
        if (value >= 0){
            vt = ValueType.POSITIVE_NUMBER;
        } else {
            vt = ValueType.NEGATIVE_NUMBER;
            value *= -1;
        }
        write(tag, vt);
        VarInt vi = new VarInt(buffer, position);
        position += vi.serialize(value);
    }

    public void add(int tag, int value) {
        add(tag, (long) value);
    }

    // Only throws if your JRE doens't support UTF-8, which it really should
    public void add(int tag, String value) throws java.io.UnsupportedEncodingException {
        write(tag, ValueType.STRING);
        byte[] serializedData = value.getBytes("UTF-8");
        VarInt vi = new VarInt(buffer, position);
        position += vi.serialize(serializedData.length);
        System.arraycopy(serializedData, 0, buffer, position, serializedData.length);
        position += serializedData.length;
    }

    public void add(int tag, byte[] value) {
        write(tag, ValueType.BYTEARRAY);
        VarInt vi = new VarInt(buffer, position);
        position += vi.serialize(value.length);
        System.arraycopy(value, 0, buffer, position, value.length);
        position += value.length;
    }

    public void add(int tag, boolean value) {
        write(tag, value ? ValueType.BOOL_TRUE : ValueType.BOOL_FALSE);
    }

    public int getPosition() {
        return position;
    }

    private byte[] buffer = null;
    private int position = -1;

    private void write(int tag, ValueType type) {
        if (tag >= 31) { // use more than 1 byte
            byte b = (byte) (type.getValue() | 0xF8); // set the 'tag' to all 1s
            buffer[position] = b;
            VarInt vi = new VarInt(buffer, position + 1);
            position += vi.serialize(tag) + 1;
            return;
        }
        assert(tag < 32);
        byte b = (byte) tag;
        b = (byte) (b << 3);
        b += type.getValue();
        buffer[position++] = b;
    }
}

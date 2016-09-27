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

import java.io.StreamCorruptedException;

public class VarInt
{
    public VarInt(byte[] data, int positionInStream) {
        this.data = data;
        position = positionInStream;
    }

    /// After doing a serialize or deserialize the position will have updated.
    public int getPosition() {
        return position;
    }

    public long unserialize() throws StreamCorruptedException {
        long result = 0;
        assert(data != null);
        assert(position >= 0);
        int pos = position;
        while (pos - position < 8 && pos < data.length) {
            byte b = data[pos++];
            result = (result << 7) | (b & 0x7F);
            if ((b & 0x80) != 0)
                result++;
            else {
                position = pos;
                return result;
            }
        }
        throw new StreamCorruptedException("Reading VarInt past stream-size");
    }

    /**
     * Append to the data block the serialized version of value, returning the amount of bytes used.
     */
    public int serialize(long value) {
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

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

public enum ValueType
{
    POSITIVE_NUMBER(0), // var-int-encoded (between 1 and 9 bytes in length). Per definition a positive number.
    NEGATIVE_NUMBER(1), // var-int-encoded (between 1 and 9 bytes in length). Per definition a negative number.
    STRING(2),          // first an UnsignedNumber for the length, then the actual bytes. Never a closing zero. Utf8 encoded.
    BYTEARRAY(3),       // identical to String, but without encoding.
    BOOL_TRUE(4),       // not followed with any bytes
    BOOL_FALSE(5);      // not followed with any bytes

    private final int value;
    private ValueType(int value) {
        this.value = value;
    }

    public int getValue() {
        return value;
    }
}

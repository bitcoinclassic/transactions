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
package transaction;

/**
 * Please refer to the online docs for the full description of the tags;
 * https://github.com/bitcoinclassic/documentation/blob/master/spec/transactionv4.md
 */
public enum TransactionFormat
{
    TX_END(0),                 // BoolTrue
    TX_IN_PREVIOUS_HASH(1),    // sha256 (Bytearray)
    TX_IN_PREVIOUS_INDEX(2),   // PositiveNumber
    TX_IN_PREVIOUS_HEIGHT(3),  // PositiveNumber
    TX_IN_SCRIPT(4),           // bytearray
    TX_OUT_VALUE(5),           // PositiveNumber (in satoshis)
    TX_OUT_SCRIPT(6),          // bytearray
    LOCK_BY_BLOCK(7),          // PositiveNumber
    LOCK_BY_TIME(8),           // PositiveNumber
    SCRIPT_VERSION(9);         // PositiveNumber

    private final int value;
    private TransactionFormat(int value) {
        this.value = value;
    }

    public int getValue() {
        return value;
    }
};

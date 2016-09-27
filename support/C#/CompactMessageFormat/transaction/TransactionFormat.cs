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

namespace CMF
{
	/**
	 * Please refer to the online docs for the full description of the tags;
	 * https://github.com/bitcoinclassic/documentation/blob/master/spec/transactionv4.md
	 */
	public enum TransactionFormat
	{
		TxEnd = 0,          // BoolTrue
		TxInPrevHash,       // sha256 (Bytearray)
		TxInPrevIndex,      // PositiveNumber
		TxInPrevHeight,     // PositiveNumber
		TxInScript,         // bytearray
		TxOutValue,         // PositiveNumber (in satoshis)
		TxOutScript,        // bytearray
		LockByBlock,        // PositiveNumber
		LockByTime,         // PositiveNumber
		ScriptVersion       // PositiveNumber
	}
}

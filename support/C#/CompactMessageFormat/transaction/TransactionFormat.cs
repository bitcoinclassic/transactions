using System;

namespace CMF
{
	/**
	 * Please refer to the online docs for the full description of the tags;
	 * https://github.com/bitcoinclassic/documentation/blob/master/spec/transactionv4.md
	 */
	public enum TransactionFormat
	{
		TX_END = 0,                 // BoolTrue
		TX_IN_PREVIOUS_HASH = 1,    // sha256 (Bytearray)
		TX_IN_PREVIOUS_INDEX = 2,   // PositiveNumber
		TX_IN_PREVIOUS_HEIGHT = 3,  // PositiveNumber
		TX_IN_SCRIPT = 4,           // bytearray
		TX_OUT_VALUE = 5,           // PositiveNumber (in satoshis)
		TX_OUT_SCRIPT = 6,          // bytearray
		LOCK_BY_BLOCK = 7,          // PositiveNumber
		LOCK_BY_TIME = 8,           // PositiveNumber
		SCRIPT_VERSION = 9          // PositiveNumber
	}
}

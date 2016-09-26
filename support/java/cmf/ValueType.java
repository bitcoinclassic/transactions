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

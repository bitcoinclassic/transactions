package cmf;

import java.nio.*;

public class MessageParser
{
    public MessageParser(byte[] data, int position, int length) {
        this.data = data;
        this.position = position;
        endPosition = position + length;
        assert(data.length >= endPosition);
    }

    public enum State {
        FOUND_TAG,
        END_OF_DOCUMENT,
        ERROR
    };

    public State next() {
        if (endPosition <= position)
            return State.END_OF_DOCUMENT;

        int b = data[position];
        if (b < 0) //  we use unsigned
            b += 256;

        ValueType type = ValueType.values()[b & 0x07];

        tag = b >> 3;
        if (tag == 31) { // the tag is stored in the next byte(s)
            try {
                VarInt vi = new VarInt(data, ++position);
                long newTag = vi.unserialize();
                if (newTag > 0xFFFF) {
                    System.out.println("Malformed tag-type "+ newTag + " is a too large enum value");
                    return State.END_OF_DOCUMENT;
                }
                position = vi.getPosition() - 1;
                tag = (int) newTag;
            } catch (Exception e) {
                return State.END_OF_DOCUMENT;
            }
        }

        switch (type) {
        case NEGATIVE_NUMBER:
        case POSITIVE_NUMBER:
            try {
                VarInt vi = new VarInt(data, ++position);
                long value = vi.unserialize();
                position = vi.getPosition();
                if (type == ValueType.NEGATIVE_NUMBER)
                    value *= -1;
                this.value = new Long(value);
            } catch (Exception e) {
                return State.END_OF_DOCUMENT;
            }
            break;
        case BYTEARRAY:
        case STRING:
            try {
                VarInt vi = new VarInt(data, ++position);
                int length = (int) vi.unserialize();
                if (vi.getPosition() + length > data.length) // need more bytes
                    return State.END_OF_DOCUMENT;

                valueState = type == ValueType.BYTEARRAY ? Lazy.ByteArray : Lazy.String;
                dataStart = vi.getPosition();
                dataLength = length;
                position = dataStart + length;
            } catch (Exception e) {
                return State.END_OF_DOCUMENT;
            }
            break;
        case BOOL_TRUE:
            value = Boolean.TRUE;
            ++position;
            break;
        case BOOL_FALSE:
            value = Boolean.FALSE;
            ++position;
            break;
        default:
            return State.ERROR;
        }
        return State.FOUND_TAG;
    }

    public int getTag() {
        return tag;
    }

    public int consumed() {
        return position;
    }

    public void consume(int bytes) {
        assert(bytes >= 0);
        position =+ bytes;
    }

    public int getInt() throws ClassCastException, NumberFormatException {
        Long object = (Long) value;
        long l = (long)object;
        if (l > Integer.MAX_VALUE)
            throw new NumberFormatException("Too big");
        return (int) l;
    }

    public long getLong() throws ClassCastException {
        Long object = (Long) value;
        return (long) object;
    }

    public boolean getBoolean() throws ClassCastException {
        Boolean object = (Boolean) value;
        return (boolean) object;
    }

    public String getString() throws ClassCastException {
        if (valueState == Lazy.String) {
            String answer = new String(data, dataStart, dataLength,
                    java.nio.charset.Charset.forName("UTF-8"));
            value = answer;
            valueState = Lazy.Parsed;
            return answer;
        }
        return (String) value;
    }

    public byte[] getByteArray() throws ClassCastException {
        if (valueState == Lazy.ByteArray) {
            byte[] answer = new byte[dataLength];
            System.arraycopy(data, dataStart, answer, 0, dataLength);
            value = answer;
            return answer;
        }
        return new byte[0];
    }

    private byte[] data = null;
    private int position = 0;
    private int endPosition = 0;
    private int tag = 0;

    private enum Lazy {
        Parsed,
        ByteArray,
        String
    };
    private Lazy valueState = Lazy.Parsed;
    private int dataStart = -1;
    private int dataLength = -1;
    private Object value = null;
}



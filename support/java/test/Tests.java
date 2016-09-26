package test;

import static org.junit.Assert.assertEquals;
import org.junit.Test;

import cmf.*;
import java.nio.ByteBuffer;

public class Tests {
    @Test
    public void basicTest() {
        byte[] buffer = new byte[100];
        {
            MessageBuilder builder = new MessageBuilder(buffer, 0);
            builder.add(15, 6512);
            assertEquals(3, builder.getPosition());
        }
        assertEquals(120, buffer[0]);
        assertEquals((byte) 177, buffer[1]);
        assertEquals(112, buffer[2]);

        {
            MessageParser parser = new MessageParser(buffer, 0, 3);
            assertEquals(MessageParser.State.FOUND_TAG, parser.next());
            assertEquals(15, parser.getTag());
            assertEquals(6512, parser.getInt());
            assertEquals(MessageParser.State.END_OF_DOCUMENT, parser.next());
        }

        {
            MessageBuilder builder = new MessageBuilder(buffer, 0);
            builder.add(129, 6512);
            assertEquals(5, builder.getPosition());
        }
        assertEquals((byte) 248, buffer[0]);
        assertEquals((byte) 128, buffer[1]);
        assertEquals(1, buffer[2]);
        assertEquals((byte) 177, buffer[3]);
        assertEquals((byte) 112, buffer[4]);

        {
            MessageParser parser = new MessageParser(buffer, 0, 5);
            assertEquals(MessageParser.State.FOUND_TAG, parser.next());
            assertEquals(129, parser.getTag());
            assertEquals(6512, parser.getInt());
            assertEquals(MessageParser.State.END_OF_DOCUMENT, parser.next());
        }
    }

    @Test
    public void testTypes() throws Exception
    {
        byte[] buffer = new byte[100];
        MessageBuilder builder = new MessageBuilder(buffer, 0);
        builder.add(1, "Föo");
        byte[] blob = new String("hihi").getBytes("UTF-8");

        builder.add(200, blob);
        builder.add(3, true);
        builder.add(40, false);

        assertEquals(17, builder.getPosition());

        // string '1'
        assertEquals((byte) 10, buffer[0]);
        assertEquals((byte) 4, buffer[1]); // serialized string length
        assertEquals((byte) 70, buffer[2]);
        assertEquals((byte) 195, buffer[3]);
        assertEquals((byte) 182, buffer[4]);
        assertEquals((byte) 111, buffer[5]);

        // blob '200'
        assertEquals((byte) 251, buffer[6]);
        assertEquals((byte) 128, buffer[7]);
        assertEquals((byte) 72, buffer[8]);
        assertEquals((byte) 4, buffer[9]); // length of bytearray
        assertEquals((byte) 104, buffer[10]); // 'h'
        assertEquals((byte) 105, buffer[11]); // 'i'
        assertEquals((byte) 104, buffer[12]); // 'h'
        assertEquals((byte) 105, buffer[13]); // 'i'

        // bool-true '3'
        assertEquals((byte) 28, buffer[14]);

        // bool-false '40'
        assertEquals((byte) 253, buffer[15]);
        assertEquals((byte) 40, buffer[16]);

        MessageParser parser = new MessageParser(buffer, 0, builder.getPosition());
        assertEquals(MessageParser.State.FOUND_TAG, parser.next());
        assertEquals(1, parser.getTag());
        assertEquals(new String("Föo"), parser.getString());
        assertEquals(MessageParser.State.FOUND_TAG, parser.next());
        assertEquals(200, parser.getTag());
        byte[] parsed = parser.getByteArray();
        assertEquals(blob.length, parsed.length);
        for (int i = 0; i < blob.length; ++i) {
            assertEquals(blob[i], parsed[i]);
        }
        assertEquals(MessageParser.State.FOUND_TAG, parser.next());
        assertEquals(3, parser.getTag());
        assertEquals(true, parser.getBoolean());
        assertEquals(MessageParser.State.FOUND_TAG, parser.next());
        assertEquals(40, parser.getTag());
        assertEquals(false, parser.getBoolean());
        assertEquals(MessageParser.State.END_OF_DOCUMENT, parser.next());
    }
}

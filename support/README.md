# Language support for transactions and CMF

Using the Flexible Transactions technology outside of the Bitcoin C++
project is something that should be made as smooth as possible and as a
result this project holds classes to allow creating / parsing of any sort
of transaction using the basic building blocks of the Compact Message
Format (CMF) combined with the enum TransactionsFormat.

In all languages you will find the MessageBuilder and the MessageParser
classes which are your simple and direct interface to the compact message
format.

The builder is rather simple in usage where you do something like;

<pre>
  byte[] bytes = new byte[100];
  MessageBuilder builder = new MessageBuilder(bytes);
  builder.add(TransactionFormat.TxOutValue, valueInSatoshis);
</pre>

This allows really easy to read and understand code.

The MessageParser is using more of a SOX parser approach where you call
`MessageParser.Next()` and then you can ask the parser for the tag-is and
the actual value.

At this time there are implementations for;

* C++ which depend on Qt
* C# Should work with any version, the project assumes 4.5
* Java

# Library details

The CompactMessageFormat (CMF) namespace contains various tools to be able to stream
data in binary form to allow compact storage as well as super fast parsing while not
throwing away advantages found in formats like XML/JSON to add new fields or types
at a later time.
Full spec at: https://github.com/bitcoinclassic/documentation/blob/master/spec/compactmessageformat.md

The format is optimized for size.
Each row contains at least 1 byte that is a combination of a 'name' and a data-type.
The name is actually a user-supplied enum and it is useful to keep that enum value within
5 bits to avoid overflows. This is no problem in practice, just use different enums in
different contexts.

The first byte also contains the type in its lowest 3 bits. This is described
in CMF.ValueType
For each type there may be zero or many bytes following the first byte. From zero bytes
for the boolean types (useful as separators) to variable sized fields for bytearrays and
strings.

Notice how practically everywhere we use variable-size integers with a maximum bit size
of 64 bits. It is assumed that that is the maximum size integers people use, otherwise its
just going to be a byte-array.


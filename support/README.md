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

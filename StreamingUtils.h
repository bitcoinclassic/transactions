/*
 * This file is part of the Bitcoin project
 * Copyright (C) 2014-2016 Tom Zander <tomz@freedommail.ch>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <QByteArray>
#include <QVariant>
#include <QPointF>

class QIODevice;
class QString;

/**
 * The streaming namespace contains various tools to be able to stream data in binary form
 * to allow compact storage as well as super fast parsing while not throwing away advantages
 * found in formats like XML/JSON to add new fields or types at a later time.
 *
 * The format is optimized for size.
 * Each row contains at least 1 byte that is a combination of a 'name' and a data-type.
 * The name is actually a user-supplied enum and it is useful to keep that enum value within
 * 5 bits to avoid overflows. This is no problem in practice, just use different enums in
 * different contexts.
 *
 * The first byte also contains the type in its lowest 3 bits. This is described
 * in Streaming::ValueType
 * For each type there may be zero or many bytes following the first byte. From zero bytes
 * for the boolean types (useful as separators) to variable sized fields for bytearrays and
 * strings.
 *
 * Notice how practically everywhere we use variable-size integers with a maximum bit size
 * of 64 bits. It is assumed that that is the maximum size integers people use, otherwise its
 * just going  to be a byte-array.
 */
namespace Streaming {
    enum ValueType {
        PositiveNumber = 0, // var-int-encoded (between 1 and 9 bytes in length). Per definition a positive number.
        NegativeNumber = 1, // var-int-encoded (between 1 and 9 bytes in length). Per definition a negative number.
        String = 2,         // first an UnsignedNumber for the length, then the actual bytes. Never a closing zero. Utf8 encoded.
        ByteArray = 3,      // identical to String, but without encoding.
        BoolTrue = 4,       // not followed with any bytes
        BoolFalse = 5       // not followed with any bytes
    };
}


/**
 * Message builder creates a generic message in the form of
 *    name: value
 * pairs, in a flat list.
 *
 * The unique part is that the value is typed and for variable-length structures
 * (like a string) a length is included.
 * The effect is that you can fully parse a structure without having any prior knowledge
 * of the fields, the expected content in the fields and the size.
 * You can compare this to an XML stream where some items are stored with tags or attributes
 * are unknown to the reader, without causing any effect on being able to parse them or to write
 * them out again unchanged.
 */
class MessageBuilder
{
public:
    explicit MessageBuilder(QIODevice *device);
    explicit MessageBuilder(QByteArray *byteArray);

    ~MessageBuilder();

    void add(quint32 tag, quint64 value);
    void add(quint32 tag, qint64 value);
    inline void add(quint32 tag, qint32 value) {
        add(tag, (qint64) value);
    }
    void add(quint32 tag, const QString &value);
    void add(quint32 tag, const QByteArray &data);
    void add(quint32 tag, const QPointF &data);
    void add(quint32 tag, bool value);

    void close();

private:
    QIODevice *m_device;
    bool m_ownsBuffer;
    char m_data[20];
};

class MessageParser
{
public:
    MessageParser(const QByteArray &data);

    enum Type {
        FoundTag,
        EndOfDocument,
        Error
    };

    Type next();

    quint32 tag() const;
    QVariant data();

    /// return the amount of bytes consumed up-including the latest parsed tag.
    inline int consumed() const {
        return m_position;
    }

    /// consume a number of bytes without parsing.
    void consume(int bytes);

private:
    const QByteArray m_data;
    const char *m_privData;
    int m_position;
    bool m_validTagFound;
    QVariant m_value;
    quint32 m_tag;

    enum LazyState {
        ValueParsed,
        LazyByteArray,
        LazyString
    };
    LazyState m_valueState;
    int m_dataStart;
    int m_dataLength;
};

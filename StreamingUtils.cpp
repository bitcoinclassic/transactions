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
#include "StreamingUtils.h"
#include "StreamMethods.h"

#include <QBuffer>
#include <QDebug>

namespace {
    int write(char *data, quint32 tag, Streaming::ValueType type) {
        Q_ASSERT(type < 8);
        if (tag >= 31) { // use more than 1 byte
            quint8 byte = type | 0xF8; // set the 'tag' to all 1s
            data[0] = byte;
            return Streaming::serialize(data +1, tag) + 1;
        }
        else {
            Q_ASSERT(tag < 32);
            quint8 byte = tag;
            byte = byte << 3;
            byte += type;
            data[0] = byte;
            return 1;
        }
    }
}

MessageBuilder::MessageBuilder(QIODevice *device)
    : m_device(device),
      m_ownsBuffer(false)
{
    Q_ASSERT(device);
}

MessageBuilder::MessageBuilder(QByteArray *byteArray)
    : m_device(new QBuffer(byteArray)),
      m_ownsBuffer(true)
{
    Q_ASSERT(byteArray);
    m_device->open(QIODevice::WriteOnly);
}

MessageBuilder::~MessageBuilder()
{
    if (m_ownsBuffer) {
        qobject_cast<QBuffer*>(m_device)->close();
        delete m_device;
    }
}

void MessageBuilder::add(quint32 tag, qint64 value)
{
    Streaming::ValueType vt;
    if (value >= 0) {
        vt = Streaming::PositiveNumber;
    } else {
        vt = Streaming::NegativeNumber;
        value *= -1;
    }
    int tagSize = write(m_data, tag, vt);
    tagSize += Streaming::serialize(m_data + 1, value);
    m_device->write(m_data, tagSize);
}

void MessageBuilder::add(quint32 tag, quint64 value)
{
    int tagSize = write(m_data, tag, Streaming::PositiveNumber);
    tagSize += Streaming::serialize(m_data + 1, value);
    m_device->write(m_data, tagSize);
}

void MessageBuilder::add(quint32 tag, const QString &value)
{
    int tagSize = write(m_data, tag, Streaming::String);
    const QByteArray serializedData = value.toUtf8();
    tagSize += Streaming::serialize(m_data + 1, serializedData.count());
    m_device->write(m_data, tagSize);
    m_device->write(serializedData);
}

void MessageBuilder::add(quint32 tag, const QByteArray &data)
{
    int tagSize = write(m_data, tag, Streaming::ByteArray);
    tagSize += Streaming::serialize(m_data + 1, data.count());
    m_device->write(m_data, tagSize);
    m_device->write(data);
}

void MessageBuilder::add(quint32 tag, bool value)
{
    write(m_data, tag, value ? Streaming::BoolTrue : Streaming::BoolFalse);
    m_device->write(m_data, 1);
}

void MessageBuilder::close()
{
    if (m_ownsBuffer)
        qobject_cast<QBuffer*>(m_device)->close();
}

///////////////////////////////////

MessageParser::MessageParser(const QByteArray &data)
    : m_data(data),
    m_privData(m_data.data()),
    m_position(0),
    m_validTagFound(false),
    m_tag(0),
    m_valueState(ValueParsed),
    m_dataStart(-1),
    m_dataLength(-1)
{
}

MessageParser::Type MessageParser::next()
{
    if (m_data.count() <= m_position)
        return EndOfDocument;

    quint8 byte = m_privData[m_position];
    Streaming::ValueType type = static_cast<Streaming::ValueType>(byte & 0x07);

    m_tag = byte >> 3;
    if (m_tag == 31) { // the tag is stored in the next byte(s)
        quint64 newTag = 0;
        bool ok = Streaming::unserialize(m_privData, m_data.size(), ++m_position, newTag);
        if (ok && newTag > 0xFFFF) {
            qWarning() << "Malformed tag-type" << newTag << "is a too large enum value";
            ok = false;
        }
        if (!ok) {
            --m_position;
            return EndOfDocument;
        }
        m_tag = newTag;
    }

    quint64 value = 0;
    switch (type) {
    case Streaming::NegativeNumber:
    case Streaming::PositiveNumber: {
        bool ok = Streaming::unserialize(m_privData, m_data.size(), ++m_position, value);
        if (!ok) {
            --m_position;
            return EndOfDocument;
        }
        if (type == Streaming::NegativeNumber)
            value *= -1;
        m_value = QVariant(value);
        break;
    }
    case Streaming::ByteArray:
    case Streaming::String: {
        int newPos = m_position + 1;
        bool ok = Streaming::unserialize(m_privData, m_data.size(), newPos, value);
        if (!ok)
            return EndOfDocument;
        if (newPos + value > (unsigned int) m_data.count()) // need more bytes
            return EndOfDocument;

        m_valueState = type == Streaming::ByteArray ? LazyByteArray : LazyString;
        m_dataStart = newPos;
        m_dataLength = value;
        m_position = newPos + value;
        break;
    }
    case Streaming::BoolTrue:
        m_value = QVariant(true);
        ++m_position;
        break;
    case Streaming::BoolFalse:
        m_value = QVariant(false);
        ++m_position;
        break;
    default:
        return Error;
    }
    return FoundTag;
}

quint32 MessageParser::tag() const
{
    return m_tag;
}

QVariant MessageParser::data()
{
    if (m_valueState != ValueParsed) {
        QByteArray data(m_privData + m_dataStart, m_dataLength);
        Q_ASSERT(data.count() == (int) m_dataLength);
        if (m_valueState == LazyByteArray)
            m_value = QVariant(data);
        else
            m_value = QVariant(QString::fromUtf8(data));
        m_valueState = ValueParsed;
    }

    return m_value;
}

void MessageParser::consume(int bytes)
{
    Q_ASSERT(bytes >= 0);
    m_position += bytes;
}

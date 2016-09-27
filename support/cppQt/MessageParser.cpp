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
#include "MessageParser.h"
#include "CMF.h"

#include <QBuffer>
#include <QDebug>

MessageParser::MessageParser(const QByteArray &data)
    : m_data(data),
    m_privData(m_data.data()),
    m_position(0),
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
    CMF::ValueType type = static_cast<CMF::ValueType>(byte & 0x07);

    m_tag = byte >> 3;
    if (m_tag == 31) { // the tag is stored in the next byte(s)
        quint64 newTag = 0;
        bool ok = CMF::unserialize(m_privData, m_data.size(), ++m_position, newTag);
        if (ok && newTag > 0xFFFF) {
            qWarning() << "Malformed tag-type" << newTag << "is a too large enum value";
            ok = false;
        }
        --m_position;
        if (!ok)
            return Error;
        m_tag = newTag;
    }

    quint64 value = 0;
    switch (type) {
    case CMF::NegativeNumber:
    case CMF::PositiveNumber: {
        bool ok = CMF::unserialize(m_privData, m_data.size(), ++m_position, value);
        if (!ok) {
            --m_position;
            return Error;
        }
        if (type == CMF::NegativeNumber)
            value *= -1;
        m_value = QVariant(value);
        break;
    }
    case CMF::ByteArray:
    case CMF::String: {
        int newPos = m_position + 1;
        bool ok = CMF::unserialize(m_privData, m_data.size(), newPos, value);
        if (!ok)
            return Error;
        if (newPos + value > (unsigned int) m_data.count()) // need more bytes
            return Error;

        m_valueState = type == CMF::ByteArray ? LazyByteArray : LazyString;
        m_dataStart = newPos;
        m_dataLength = value;
        m_position = newPos + value;
        break;
    }
    case CMF::BoolTrue:
        m_value = QVariant(true);
        ++m_position;
        break;
    case CMF::BoolFalse:
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

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
#include "MessageBuilder.h"
#include "CMF.h"

#include <QBuffer>
#include <QDebug>

namespace {
    int write(char *data, quint32 tag, CMF::ValueType type) {
        Q_ASSERT(type < 8);
        if (tag >= 31) { // use more than 1 byte
            quint8 byte = type | 0xF8; // set the 'tag' to all 1s
            data[0] = byte;
            return CMF::serialize(data +1, tag) + 1;
        }
        Q_ASSERT(tag < 32);
        quint8 byte = tag;
        byte = byte << 3;
        byte += type;
        data[0] = byte;
        return 1;
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
    CMF::ValueType vt;
    if (value >= 0) {
        vt = CMF::PositiveNumber;
    } else {
        vt = CMF::NegativeNumber;
        value *= -1;
    }
    int tagSize = write(m_data, tag, vt);
    tagSize += CMF::serialize(m_data + tagSize, value);
    m_device->write(m_data, tagSize);
}

void MessageBuilder::add(quint32 tag, quint64 value)
{
    int tagSize = write(m_data, tag, CMF::PositiveNumber);
    tagSize += CMF::serialize(m_data + tagSize, value);
    m_device->write(m_data, tagSize);
}

void MessageBuilder::add(quint32 tag, const QString &value)
{
    int tagSize = write(m_data, tag, CMF::String);
    const QByteArray serializedData = value.toUtf8();
    tagSize += CMF::serialize(m_data + tagSize, serializedData.count());
    m_device->write(m_data, tagSize);
    m_device->write(serializedData);
}

void MessageBuilder::add(quint32 tag, const QByteArray &data)
{
    int tagSize = write(m_data, tag, CMF::ByteArray);
    tagSize += CMF::serialize(m_data + tagSize, data.count());
    m_device->write(m_data, tagSize);
    m_device->write(data);
}

void MessageBuilder::add(quint32 tag, bool value)
{
    int tagSize = write(m_data, tag, value ? CMF::BoolTrue : CMF::BoolFalse);
    m_device->write(m_data, tagSize);
}

void MessageBuilder::close()
{
    if (m_ownsBuffer)
        qobject_cast<QBuffer*>(m_device)->close();
}

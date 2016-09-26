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
#ifndef MESSAGEBUILDER_H
#define MESSAGEBUILDER_H

#include <QByteArray>
#include <QVariant>
#include <QPointF>

class QIODevice;
class QString;


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

#endif

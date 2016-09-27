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
#ifndef MESSAGEPARSER_H
#define MESSAGEPARSER_H

#include <QByteArray>
#include <QVariant>
#include <QPointF>

class QIODevice;
class QString;

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

#endif

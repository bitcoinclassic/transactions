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

/**
 * MessageParser takes a CMF stream and parses it into tokens.
 *
 * A user can iterate over tokens by repeatedly calling next() and as
 * long as FoundTag is returned you can then find the token-tag
 * as well as the actual data. The MessageParser will actually have typed
 * data based on what was encoded in the CMF stream. You can access
 * this using the different getters like getLong() getString(). Please note
 * that if the requested data is not what was present in the stream you will
 * end up throwing a casting exception in those getters.
 */
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

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
#ifndef STREAMMETHODS_H
#define STREAMMETHODS_H

#include <QByteArray>

namespace Streaming {

inline unsigned int fetch32bitValue(const char *array, int offset)
{
    unsigned int answer = static_cast<unsigned char>(array[offset + 3]);
    answer = answer << 8;
    answer += static_cast<unsigned char>(array[offset + 2]);
    answer = answer << 8;
    answer += static_cast<unsigned char>(array[offset + 1]);
    answer = answer << 8;
    answer += static_cast<unsigned char>(array[offset]);
    return answer;
}

void insert32BitInt(QByteArray &array, quint32 value, int pos);

inline quint64 fetch64bitValue(const char *array, int offset)
{
    quint64 answer = fetch32bitValue(array, offset + 4);
    answer  = answer << 32;
    answer += fetch32bitValue(array, offset);
    return answer;
}

static inline unsigned int fetch16bitValue(const char *array, int offset)
{
    unsigned int answer = static_cast<unsigned char>(array[offset + 1]);
    answer = answer << 8;
    answer += static_cast<unsigned char>(array[offset]);
    return answer;
}

// Bitcoin has its own, kinda braindead, var-int "compact" format.
// Its not very compact, though.
inline quint64 fetchBitcoinCompact(const char *array, int &offset)
{
    int indicator = (unsigned char) array[offset++];
    if (indicator < 253)
        return indicator;
    if (indicator == 253) {
        unsigned int answer = fetch16bitValue(array, offset);
        offset += 2;
        return answer;
    }
    if (indicator == 254)
    {
        unsigned int answer = fetch32bitValue(array, offset);
        offset += 4;
        return answer;
    }
    quint64 answer = fetch64bitValue(array, offset);
    offset += 8;
    return answer;
}

}

#endif

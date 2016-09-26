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
#include "CMF.h"

int CMF::serialize(char *data, quint64 value)
{
    int pos = 0;
    while (true) {
        data[pos] = (value & 0x7F) | (pos ? 0x80 : 0x00);
        if (value <= 0x7F)
            break;
        value = (value >> 7) - 1;
        pos++;
    }

    // reverse
    for (int i = pos / 2; i >= 0; --i) {
        qSwap(data[i], data[pos - i]);
    }
    return pos + 1;
}

bool CMF::unserialize(const char *data, int dataSize, int &position, quint64 &result)
{
    Q_ASSERT(data);
    Q_ASSERT(result == 0);
    Q_ASSERT(position >= 0);
    int pos = position;
    while (pos - position < 8 && pos < dataSize) {
        unsigned char byte = data[pos++];
        result = (result << 7) | (byte & 0x7F);
        if (byte & 0x80)
            result++;
        else {
            position = pos;
            return true;
        }
    }
    return false;
}

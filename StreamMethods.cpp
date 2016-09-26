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
#include "StreamMethods.h"

void Streaming::insert32BitInt(QByteArray &array, quint32 value, int pos)
{
    Q_ASSERT(pos >= 0);
    const char a = static_cast<char>((value >> 24) & 0xFF);
    const char b = static_cast<char>((value >> 16) & 0xFF);
    const char c = static_cast<char>((value >> 8) & 0xFF);
    const char d = static_cast<char>(value & 0xFF);
    array[pos] = d;
    array[pos + 1] = c;
    array[pos + 2] = b;
    array[pos + 3] = a;
}

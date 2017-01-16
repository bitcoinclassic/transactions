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

#include <qglobal.h>

/**
 * The CompactMessageFormat (CMF) namespace contains various tools to be able to stream
 * data in binary form to allow compact storage as well as super fast parsing while not
 * throwing away advantages found in formats like XML/JSON to add new fields or types
 * at a later time.
 * Full spec at: https://github.com/bitcoinclassic/documentation/blob/master/spec/compactmessageformat.md
 *
 * The format is optimized for size.
 * Each row contains at least 1 byte that is a combination of a 'name' and a data-type.
 * The name is actually a user-supplied enum and it is useful to keep that enum value within
 * 5 bits to avoid overflows. This is no problem in practice, just use different enums in
 * different contexts.
 *
 * The first byte also contains the type in its lowest 3 bits. This is described
 * in CMF::ValueType
 * For each type there may be zero or many bytes following the first byte. From zero bytes
 * for the boolean types (useful as separators) to variable sized fields for bytearrays and
 * strings.
 *
 * Notice how practically everywhere we use variable-size integers with a maximum bit size
 * of 64 bits. It is assumed that that is the maximum size integers people use, otherwise its
 * just going  to be a byte-array.
 */
namespace CMF {
    enum ValueType {
        PositiveNumber = 0, // var-int-encoded (between 1 and 9 bytes in length). Per definition a positive number.
        NegativeNumber = 1, // var-int-encoded (between 1 and 9 bytes in length). Per definition a negative number.
        String = 2,         // first an UnsignedNumber for the length, then the actual bytes. Never a closing zero. Utf8 encoded.
        ByteArray = 3,      // identical to String, but without encoding.
        BoolTrue = 4,       // not followed with any bytes
        BoolFalse = 5       // not followed with any bytes
    };

    /// returns amount of bytes the output is
    int serialize(char *data, quint64 value);

    /**
     * take input data, which is of size dataSize and unserialize a utf8 encoded unsigned integer into result.
     */
    bool unserialize(const char *data, int dataSize, int &position, quint64 &result);
}

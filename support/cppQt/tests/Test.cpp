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
#include "Test.h"

#include "../MessageBuilder.h"
#include "../MessageParser.h"

void Test::testBasis()
{
    QByteArray bytes;
    {
        MessageBuilder builder(&bytes);
        builder.add(15, 6512);
    }
    QCOMPARE(bytes.size(), 3);
    QVERIFY(bytes.at(0) == 120);
    QVERIFY(static_cast<unsigned char>(bytes.at(1)) == 177);
    QVERIFY(bytes.at(2) == 112);

    {
        MessageParser parser(bytes);
        MessageParser::Type t = parser.next();
        QCOMPARE(t, MessageParser::FoundTag);
        QCOMPARE(parser.tag(), (unsigned int) 15);
        QCOMPARE(parser.data().toInt(), 6512);
        t = parser.next();
        QCOMPARE(t, MessageParser::EndOfDocument);
    }

    {
        MessageBuilder builder(&bytes);
        builder.add(129, 6512);
    }
    QCOMPARE(bytes.size(), 5);
    QVERIFY(static_cast<unsigned char>(bytes.at(0)) == 248);
    QVERIFY(static_cast<unsigned char>(bytes.at(1)) == 128);
    QVERIFY(static_cast<unsigned char>(bytes.at(2)) == 1);
    QVERIFY(static_cast<unsigned char>(bytes.at(3)) == 177);
    QVERIFY(static_cast<unsigned char>(bytes.at(4)) == 112);

    {
        MessageParser parser(bytes);
        QCOMPARE(parser.next(), MessageParser::FoundTag);
        QCOMPARE(parser.tag(), (unsigned int) 129);
        QCOMPARE(parser.data().toInt(), 6512);
        QCOMPARE(parser.next(), MessageParser::EndOfDocument);
    }
}

void Test::testTypes()
{
    QByteArray bytes;
    MessageBuilder builder(&bytes);
    builder.add(1, QString("Föo"));
    const QByteArray blob = QString("hihi").toUtf8();
    builder.add(200, blob);
    builder.add(3, true);
    builder.add(40, false);

    QCOMPARE(bytes.size(), 17);
    // string '1'
    QVERIFY(static_cast<unsigned char>(bytes.at(0)) == 10);
    QVERIFY(static_cast<unsigned char>(bytes.at(1)) == 4); // serialized string length
    QVERIFY(static_cast<unsigned char>(bytes.at(2)) == 70);
    QVERIFY(static_cast<unsigned char>(bytes.at(3)) == 195);
    QVERIFY(static_cast<unsigned char>(bytes.at(4)) == 182);
    QVERIFY(static_cast<unsigned char>(bytes.at(5)) == 111);

    // blob '200'
    QVERIFY(static_cast<unsigned char>(bytes.at(6)) == 251);
    QVERIFY(static_cast<unsigned char>(bytes.at(7)) == 128);
    QVERIFY(static_cast<unsigned char>(bytes.at(8)) == 72);
    QVERIFY(static_cast<unsigned char>(bytes.at(9)) == 4); // length of bytearray
    QVERIFY(static_cast<unsigned char>(bytes.at(10)) == 104);  //'h'
    QVERIFY(static_cast<unsigned char>(bytes.at(11)) == 105);  //'i'
    QVERIFY(static_cast<unsigned char>(bytes.at(12)) == 104);  //'h'
    QVERIFY(static_cast<unsigned char>(bytes.at(13)) == 105);  //'i'

    // bool-true '3'
    QVERIFY(static_cast<unsigned char>(bytes.at(14)) == 28);

    // bool-false '40'
    QVERIFY(static_cast<unsigned char>(bytes.at(15)) == 253);
    QVERIFY(static_cast<unsigned char>(bytes.at(16)) == 40);

    MessageParser parser(bytes);
    QCOMPARE(parser.next(), MessageParser::FoundTag);
    QCOMPARE(parser.tag(), (unsigned int) 1);
    QCOMPARE(parser.data().toString(), QString("Föo"));
    QCOMPARE(parser.next(), MessageParser::FoundTag);
    QCOMPARE(parser.tag(), (unsigned int) 200);
    QCOMPARE(parser.data().toByteArray(), blob);
    QCOMPARE(parser.next(), MessageParser::FoundTag);
    QCOMPARE(parser.tag(), (unsigned int) 3);
    QCOMPARE(parser.data().toBool(), true);
    QCOMPARE(parser.next(), MessageParser::FoundTag);
    QCOMPARE(parser.tag(), (unsigned int) 40);
    QCOMPARE(parser.data().toBool(), false);
    QCOMPARE(parser.next(), MessageParser::EndOfDocument);
}

QTEST_MAIN(Test)

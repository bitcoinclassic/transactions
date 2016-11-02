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
#include "Transaction.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QDebug>

int main(int x, char **y) {
    QCoreApplication app(x, y);
    QCoreApplication::setApplicationName("Transactions");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Transactions investigations");
    parser.addHelpOption();
    parser.addPositionalArgument("transaction", "Source transaction");
    parser.addPositionalArgument("out-with-sign", "output transaction, including witness");
    parser.addPositionalArgument("out-small", "output transaction");
    QCommandLineOption rawtx(QStringList() << "rawtx", "pass a raw hexformatted transaction instead of a filename" );
    parser.addOption(rawtx);

    QCommandLineOption debug(QStringList() << "d" << "debug", "Show content of the transaction" );
    parser.addOption(debug);

    parser.process(app);
    const QStringList args = parser.positionalArguments();
    if (args.isEmpty())
        parser.showHelp(1);

    Transaction t;
    if (parser.isSet(rawtx)) {
        QByteArray data = QByteArray::fromHex(args.at(0).toLatin1());
        t.read(data);
    } else {
        t.read(args.at(0));
    }

    if (parser.isSet(debug))
        t.debug();

    if (args.count() > 1) {
        QFileInfo fi(args[1]);
        if (fi.exists()) {
            qWarning() << "Outfile 1 exists, exiting";
            return 1;
        }
        t.writev4(args[1], true);
    }
    if (args.count() > 2) {
        QFileInfo fi(args[2]);
        if (fi.exists()) {
            qWarning() << "Outfile 2 exists, exiting";
            return 1;
        }
        t.writev4(args[2], false);
    }

    return 0;
}

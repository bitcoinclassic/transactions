/*
 * This file is part of the Bitcoin project
 * Copyright (C) 2016 Tom Zander <tomz@freedommail.ch>
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
#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <QList>
#include <QString>
#include <QTextStream>


class Transaction
{
public:
    Transaction();

    void read(const QString &filename);
    void read(const QByteArray &data);
    void writev4(const QString &filename, bool includeSignatures);

    void debug() const;
    void debugScript(const QByteArray &script, int textIndent, QTextStream &out) const;

    enum MessageTags {
        TxEnd = 0,          // BoolTrue
        TxInPrevHash,       // sha256 (Bytearray)
        TxInPrevIndex,      // PositiveNumber
        TxInPrevHeight,     // PositiveNumber
        TxInScript,         // bytearray
        TxOutValue,         // PositiveNumber (in satoshis)
        TxOutScript,        // bytearray
        LockByBlock,        // PositiveNumber
        LockByTime,         // PositiveNumber
        CoinbaseMessage,    // Bytearray
        ScriptVersion       // PositiveNumber
    };

private:
    void parseTransactionV1(const QByteArray &bytes);
    void parseTransactionV4(const QByteArray &bytes);

    int m_version;

    struct TxIn {
        TxIn() : prevIndex(0), sequence(0) {}
        TxIn(const QByteArray &prevHash) : transaction(prevHash), prevIndex(0), sequence(0) {}
        QByteArray transaction;
        int prevIndex;
        QByteArray script;
        int sequence;
    };
    struct TxOut {
        TxOut(){}
        TxOut(const QByteArray &bytes, quint64 val) : script(bytes), value(val) {}
        QByteArray script;
        quint64 value; // aka amount of satoshis
    };

    QList<TxIn> m_inputs;
    QList<TxOut> m_outputs;

    quint32 m_nLockTime;
    QByteArray m_coinbaseMessage;
};

#endif

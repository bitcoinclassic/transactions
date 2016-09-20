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
#include "Transaction.h"
#include "StreamingUtils.h"
#include "StreamMethods.h"

#include <QFile>
#include <QDebug>

Transaction::Transaction()
    : m_version(-1),
      m_nLockTime(0)
{
}

void Transaction::read(const QString &filename)
{
    qDebug() << "read" << filename;
    QFile in(filename);
    if (!in.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open input" << filename;
        return;
    }
    QByteArray bytes = in.readAll();
    in.close();
    if (bytes.isEmpty()) {
        qWarning() << "Empty input file";
        return;
    }

    if (bytes.length() <=4 || bytes.at(1) != 0 || bytes.at(2) != 0 || bytes.at(3) != 0) {
        qWarning() << "Unknown transaction format. Cowerdly bailing out before trying to parse.";
    } else if (bytes.at(0) <= 2) {
        parseTransactionV1(bytes);
    } else if (bytes.at(0) == 4) {
        m_version = 4;
        parseTransactionV4(bytes.mid(4));
    } else {
        qWarning() << "Unknown transaction version. Can't parse.";
    }
}

void Transaction::writev4(const QString &filename, bool includeSignatures)
{
    QFile out(filename);
    if (!out.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to write file" << filename;
        return;
    }

    QByteArray version;
    version.resize(4);
    Streaming::insert32BitInt(version, 4, 0);
    out.write(version);

    MessageBuilder builder(&out);
    foreach (const TxIn &tx, m_inputs) {
        builder.add(TxInPrevHash, tx.transaction);
        if (tx.prevIndex > 0)
            builder.add(TxInPrevIndex, tx.prevIndex);
    }
    foreach (const TxOut &tx, m_outputs) {
        builder.add(TxOutScript, tx.script);
        builder.add(TxOutValue, tx.value);
    }

    // This is the limiter. All the data above is used to create the transaction ID.
    // What follows is the tx-in scripts. These contain the public key (of which the txout (prevtx) has a hash)
    // and it contains a signature using that public key signing the data above.

    // This means that the data below can be stripped after we established that the transaction is correct.
    // There is no need to hash the below data to ensure its not tampered with. They are cryptographic proof
    // on their own by the fact that they 'unlock' the puzzle of the prev TX and sign this TX.

    if (includeSignatures) {
        foreach (const TxIn &tx, m_inputs) {
            builder.add(TxInScript, tx.script);
        }
        builder.add(TxEnd, true);
    }
}

bool Transaction::isValid() const
{
    return !(m_inputs.isEmpty() || m_outputs.isEmpty() || m_version <= 0);
}

void Transaction::debug() const
{
    Q_ASSERT(isValid());

    QTextStream out(stdout);
    out << "{\ninputs :[\n";
    foreach (const TxIn &tx, m_inputs) {
        out << "  {\n    txid: " << tx.transaction.toHex() << endl;
        out << "    vout: " << tx.prevIndex << endl;
        out << "    sequence: " << QString::number(tx.sequence, 16) << endl;
        out << "    script: ";
        debugScript(tx.script, 12, out);
        out << "  }\n";
    }
    out << "]\n";
    out << "outputs: [\n";
    foreach (const TxOut &tx, m_outputs) {
        out << "  {\n    amount: " << tx.value << endl;
        out << "    script: ";
        debugScript(tx.script, 12, out);
        out << "  }\n";
    }
    out << "]\nversion: " << m_version << "\nnLockTime: " << m_nLockTime << "\n}\n";
}

namespace {
void printHex(const char *data, int index, QTextStream &out)
{
    const unsigned char k= (const unsigned char)(data[index]);
    if (k < 16)
        out << '0';
    out << QString::number(k, 16);
}

int printBytes(const char *data, int offset, QTextStream &out, int bytes = -1)
{
    if (bytes <= 0) {
        bytes = (quint8) data[offset];
        offset += 1;
    }
    out << QByteArray(data + offset, bytes).toHex();
    return bytes;
}
}

void Transaction::debugScript(const QByteArray &script, int textIndent, QTextStream &out) const
{
    const int length = script.length();
    const char *data = script.constData();
    int pos = 0;
    QString indent;
    for (int i = 0; i < textIndent; ++i) indent += ' ';
    bool linefeed = false;

    while (pos < length) {
        const unsigned char k= (const unsigned char)(data[pos]);
        if (linefeed)
            out << indent;
        if (k > 0 && k < 75) {
            for (int i = 0; i < k; ++i) {
                pos++;
                printHex(data, pos, out);
            }
            linefeed = true;
        } else {
            if (!linefeed)
                out << endl << indent;
            linefeed = true;
            switch (k) {
            case 0:
                out << "OP_FALSE"; break;
            case 81:
                out << "OP_TRUE"; break;
            case 76:
            case 77:
            case 78: {
                const int width = k - 75;
                out << "OP_PUSHDATA" << width << " ";
                quint32 bytes = data[++pos];
                if (width > 1) {
                    bytes = bytes << 8;
                    bytes += data[++pos];
                    if (width > 2) {
                        bytes = bytes << 8;
                        bytes += data[++pos];
                        bytes = bytes << 8;
                        bytes += data[++pos];
                    }
                }
                for (unsigned int i = 0; i < bytes; ++i) {
                    pos++;
                    printHex(data, pos, out);
                }
                break;
            }
            case 82: case 83: case 84: case 85: case 86: case 87: case 88: case 89: case 90:
            case 91: case 92: case 93: case 94: case 95: case 96: {
                int count = k - 80;
                out << "OP_" << count;
                break;
            }
            case 97:
                out << "OP_NOP"; break;
            case 99:
                out << "OP_IF"; break;
            case 100:
                out << "OP_NOTIF"; break;
            case 103:
                out << "OP_ELSE"; break;
            case 104:
                out << "OP_ENDIF"; break;
            case 105:
                out << "OP_VERIFY"; break;
            case 106:
                out << "OP_RETURN"; break;
            case 107:
                out << "OP_TOALTSTACK"; break;
            case 108:
                out << "OP_FROMALTSTACK"; break;
            case 115:
                out << "OP_IFDUP"; break;
            case 116:
                out << "OP_DEPTH"; break;
            case 117:
                out << "OP_DROP"; break;
            case 118:
                out << "OP_DUP"; break;
            case 119:
                out << "OP_NIP"; break;
            case 120:
                out << "OP_OVER"; break;
            case 121:
                out << "OP_PICK"; break;
            case 122:
                out << "OP_ROLL"; break;
            case 123:
                out << "OP_ROT"; break;
            case 124:
                out << "OP_SWAP"; break;
            case 125:
                out << "OP_TUCK"; break;
            case 109:
                out << "OP_2DROP"; break;
            case 110:
                out << "OP_2DUP"; break;
            case 111:
                out << "OP_3DUP"; break;
            case 112:
                out << "OP_2OVER"; break;
            case 113:
                out << "OP_2ROT"; break;
            case 114:
                out << "OP_2SWAP"; break;
            case 126:
                out << "OP_CAT     [ILLEGAL!]"; break;
            case 127:
                out << "OP_SUBSTR  [ILLEGAL!]"; break;
            case 128:
                out << "OP_LEFT    [ILLEGAL!]"; break;
            case 129:
                out << "OP_RIGHT   [ILLEGAL!]"; break;
            case 130:
                out << "OP_SIZE"; break;
            case 131:
                out << "OP_INVERT  [ILLEGAL!]"; break;
            case 132:
                out << "OP_AND     [ILLEGAL!]"; break;
            case 133:
                out << "OP_OR      [ILLEGAL!]"; break;
            case 134:
                out << "OP_XOR     [ILLEGAL!]"; break;
            case 135:
                out << "OP_EQUAL"; break;
            case 136:
                out << "OP_EQUALVERIFY"; break;
            case 139:
                out << "OP_1ADD"; break;
            case 140:
                out << "OP_1SUB"; break;
            case 141:
                out << "OP_2MUL    [ILLEGAL]"; break;
            case 142:
                out << "OP_2DIV    [ILLEGAL]"; break;
            case 143:
                out << "OP_NEGATE"; break;
            case 144:
                out << "OP_ABS"; break;
            case 145:
                out << "OP_NOT"; break;
            case 146:
                out << "OP_0NOTEQUAL"; break;
            case 147:
                out << "OP_ADD"; break;
            case 148:
                out << "OP_SUB"; break;
            case 149:
                out << "OP_MUL    [ILLEGAL]"; break;
            case 150:
                out << "OP_DIV    [ILLEGAL]"; break;
            case 151:
                out << "OP_MOD    [ILLEGAL]"; break;
            case 152:
                out << "OP_LSHIFT [ILLEGAL]"; break;
            case 153:
                out << "OP_RSHIFT [ILLEGAL]"; break;
            case 154:
                out << "OP_BOOLAND"; break;
            case 155:
                out << "OP_BOOLOR"; break;
            case 156:
                out << "OP_NUMEQUAL"; break;
            case 157:
                out << "OP_NUMEQUALVERIFY"; break;
            case 158:
                out << "OP_NUMNOTEQUAL"; break;
            case 159:
                out << "OP_LESSTHAN"; break;
            case 160:
                out << "OP_GREATERTHAN"; break;
            case 161:
                out << "OP_LESSTHANOREQUAL"; break;
            case 162:
                out << "OP_GREATERTHANOREQUAL"; break;
            case 163:
                out << "OP_MIN"; break;
            case 164:
                out << "OP_MAX"; break;
            case 165:
                out << "OP_WITHIN"; break;

            case 171:
                out << "OP_CODESEPARATOR"; break;
            case 172:
                out << "OP_CHECKSIG"; break;
            case 173:
                out << "OP_CHECKSIGVERIFY"; break;
            case 174:
                out << "OP_CHECKMULTISIG"; break;
            case 175:
                out << "OP_CHECKMULTISIGVERIFY"; break;
            case 177:
                out << "OP_CHECKLOCKTIMEVERIFY"; break;
            case 178:
                out << "OP_CHECKSEQUENCEVERIFY"; break;
            case 253:
                out << "OP_PUBKEYHASH [ILLEGAL]"; break;
            case 254:
                out << "OP_PUBKEY  [ILLEGAL]"; break;
            case 255:
                out << "OP_INVALIDOPCODE [ILLEGAL]"; break;
            case 80:
                out << "OP_RESERVED"; break;
            case 98:
                out << "OP_VER"; break;
            case 101:
                out << "OP_VERIF"; break;
            case 102:
                out << "OP_VERNOTIF"; break;
            case 137:
                out << "OP_RESERVED1"; break;
            case 138:
                out << "OP_RESERVED2"; break;
            case 176: case 179: case 180: case 181: case 182: case 183: case 184: case 185:
                out << "OP_NOP{}"; break;
            case 0xA9:
                out << "OP_HASH160 ";
                pos += printBytes(data, ++pos, out);
                break;
            case 166:
                out << "OP_RIPEMD160";
                pos += printBytes(data, ++pos, out);
                break;
            case 167:
                out << "OP_SHA1";
                pos += printBytes(data, ++pos, out);
                break;
            case 168:
                out << "OP_SHA256";
                pos += printBytes(data, pos, out, 32);
                break;
            case 170:
                out << "OP_HASH256";
                pos += printBytes(data, ++pos, out);
                break;
            default:
                out << "UNKNOWN: " << QString::number(k, 16);
            }
        }
        if (linefeed)
            out << endl;
        ++pos;
    }
}

void Transaction::parseTransactionV1(const QByteArray &bytes)
{
    const int length = bytes.length();
    const char *data = bytes.constData();
    unsigned int version = Streaming::fetch32bitValue(data, 0);
    Q_ASSERT(data[0] <= 2);
    Q_ASSERT(data[1] == 0);
    Q_ASSERT(data[2] == 0);
    Q_ASSERT(data[3] == 0);
    m_version = version & 0xFF;

    if (version > 1) {
        // bip68
        Q_ASSERT(false);
    }

    int pos = 4;
    quint64 count = 0;
    if (!Streaming::unserialize(data, length, pos, count)) {
        qWarning() << "Failed to parse in-counter";
        return;
    }

    QList<TxIn> inputs;
    for (unsigned int i = 0; i < count; ++i) {
        TxIn tx;
        tx.transaction.reserve(32);
        for (int i = 0; i < 32; ++i) {/// but WHY!!!
            tx.transaction[i] = data[pos + 31 - i];
        }
        pos += 32;
        tx.prevIndex = Streaming::fetch32bitValue(data, pos);
        pos += 4;

        quint32 scriptLength = Streaming::fetchBitcoinCompact(data, pos);
        Q_ASSERT(scriptLength < (unsigned int) (length - pos));

        tx.script = QByteArray(data + pos, scriptLength);
        pos += scriptLength;
        tx.sequence = Streaming::fetch32bitValue(data, pos);
        pos += 4;
        inputs.append(tx);
        Q_ASSERT(pos < length);
    }

    count = 0;
    if (!Streaming::unserialize(data, length, pos, count)) {
        qWarning() << "Failed to parse out-counter";
        return;
    }
    Q_ASSERT(pos < length);

    QList<TxOut> outputs;
    for (unsigned int i = 0; i < count; ++i) {
        TxOut tx;
        tx.value = Streaming::fetch64bitValue(data, pos);
        // qDebug() << "value" << tx.value;
        pos += 8;

        quint32 scriptLength = Streaming::fetchBitcoinCompact(data, pos);
        Q_ASSERT(scriptLength < (unsigned int) (length - pos));

        tx.script = QByteArray(data + pos, scriptLength);
        pos += scriptLength;
        outputs.append(tx);
        Q_ASSERT(pos < length);
    }

    Q_ASSERT(pos + 4 == length);
    m_nLockTime = Streaming::fetch32bitValue(data, pos);

    m_inputs= inputs;
    m_outputs = outputs;
}

void Transaction::parseTransactionV4(const QByteArray &bytes)
{
    MessageParser parser(bytes);
    Q_ASSERT(m_inputs.isEmpty());
    Q_ASSERT(m_outputs.isEmpty());

    QList<TxIn> inputs;
    QList<TxOut> outputs;
    MessageParser::Type type = parser.next();
    int inputScriptCount = 0;
    bool storedOutValue = false, storedOutScript = false;
    qint64 outValue = 0;

    while (type == MessageParser::FoundTag) {
        switch (parser.tag()) {
        case TxEnd:
            m_inputs = inputs;
            m_outputs = outputs;
            return;
        case TxInPrevHash:
            inputs.append(TxIn(parser.data().toByteArray()));
            break;
        case TxInPrevIndex:
            if (inputs.isEmpty()) {
                qWarning() << "TxInPrevIndex seen without a TxInPrevHash before it";
                return;
            }
            inputs.last().prevIndex = parser.data().toInt();
            break;
        case TxInScript:
            if (inputScriptCount >= inputs.size()) {
                qWarning() << "Too many TxInScript tags in tx";
                return;
            }
            inputs[inputScriptCount++].script = parser.data().toByteArray();
            break;
        case TxOutValue:
            if (storedOutScript) { // add it
                outputs.last().value = parser.data().toLongLong();
                storedOutScript = storedOutValue = false;
            } else { // store it
                outValue = parser.data().toLongLong();
                storedOutValue = true;
            }
            break;
        case TxOutScript:
            outputs.append(TxOut(parser.data().toByteArray(), outValue));
            if (storedOutValue)
                storedOutValue = false;
            else
                storedOutScript = true;
            break;
        default:
         // case LockByBlock:
         // case LockByTime:
         // case ScriptVersion:
         // case TxInPrevHeight:
            break;
        }
        type = parser.next();
    }

    if (type != MessageParser::EndOfDocument) {
        qWarning() << "Failed parsing transaction, MessageParser gave error.";
    } else {
        m_inputs = inputs;
        m_outputs = outputs;
    }
}

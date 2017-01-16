TEMPLATE = app
TARGET = transactions
INCLUDEPATH += . support/cppQt

# Input
HEADERS += StreamMethods.h Transaction.h \
    CMF.h \
    MessageBuilder.h \
    MessageParser.h

SOURCES += main.cpp StreamMethods.cpp Transaction.cpp \
    CMF.cpp \
    MessageBuilder.cpp \
    MessageParser.cpp


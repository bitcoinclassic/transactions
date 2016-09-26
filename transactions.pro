TEMPLATE = app
TARGET = transactions
INCLUDEPATH += . support/cppQt

# Input
HEADERS += StreamMethods.h Transaction.h \
    support/cppQt/CMF.h \
    support/cppQt/MessageBuilder.h \
    support/cppQt/MessageParser.h

SOURCES += main.cpp StreamMethods.cpp Transaction.cpp \
    support/cppQt/CMF.cpp \
    support/cppQt/MessageBuilder.cpp \
    support/cppQt/MessageParser.cpp


QT -= core

CONFIG += c++20 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


INCLUDEPATH += /usr/local/mysql/include/

LIBS += -L/usr/local/mysql/lib -lmysqlclient

SOURCES += \
        cdbconnectpool.cpp \
        cdbmanager.cpp \
        cmysql.cpp \
        main.cpp \
        tools.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
          SysConfig.h \
          cdbconnectpool.h \
          cdbmanager.h \
          cmysql.h \
          data_type_defination.h \
          threadPool.hpp \
          tools.h

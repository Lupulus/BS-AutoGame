QT       += core gui
QT += core gui multimedia multimediawidgets concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    cgui.cpp \
    chelp.cpp \
    clogconsole.cpp \
    cparam.cpp \
    cscenario.cpp \
    cstart.cpp \
    main.cpp

HEADERS += \
    cgui.h \
    chelp.h \
    clogconsole.h \
    cparam.h \
    cscenario.h \
    cstart.h

FORMS += \
    cgui.ui \
    chelp.ui \
    clogconsole.ui \
    cparam.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Spécification de l'icone du .exe !
TARGET = Emul-AutoGame
RC_ICONS = "C:\Users\Clement\Desktop\Dossier-General\DevCoding\QtApp\BS-AutoGame\ressources\images\icon.ico"

RESOURCES += \
    ressources.qrc
# Compiler OpenCV-x.x.x avec CMake.exe ! Suivre procédure suivante : https://wiki.qt.io/How_to_setup_Qt_and_openCV_on_Windows
# Faire attention si sur Window de bien sélectionner les bonnes version/télécharger les bonnes version et Attention au ESPACE dans les chemin d'accès !
# Ne pas oublié de bien entrer les variables d'environnement system suivante dans PATH :
# D:\Qt\6.8.0\mingw_64\bin
# C:\Users\Clement\Desktop\Dossier-General\DevCoding\OpenCV-4.9.0MinGW\install\lib

# Penser à mettre les *.dll du dossier /opencvdir/install/bin dans /install/lib avec les *.dll.a !
# PATH et Bibliothèque de OpenCV (Essentiel!), méthode fonctionnel...(long!)
INCLUDEPATH += "C:\Users\Clement\Desktop\Dossier-General\DevCoding\OpenCV-4.9.0MinGW\install\include"
LIBS += $$files("C:/Users/Clement/Desktop/Dossier-General/DevCoding/OpenCV-4.9.0MinGW/install/x64/mingw/lib/*.a")
LIBS += $$files("C:/Users/Clement/Desktop/Dossier-General/DevCoding/OpenCV-4.9.0MinGW/install/x64/mingw/bin/*.lib")

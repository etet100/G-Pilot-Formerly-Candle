// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#include <QApplication>
#include <QDebug>
#include <QGLWidget>
#include <QLocale>
#include <QTranslator>
#include <QFile>
#include <QStyleFactory>
#include <QFontDatabase>
#include <QMessageBox>

#include "frmmain.h"

void messageHandler(QtMsgType type, const QMessageLogContext &, const QString & msg)
{
    QString txt;
    switch (type) {
        case QtDebugMsg:
            txt = QString("Debug: %1").arg(msg);
            break;
        case QtWarningMsg:
            txt = QString("Warning: %1").arg(msg);
            break;
        case QtCriticalMsg:
            txt = QString("Critical: %1").arg(msg);
            break;
        case QtInfoMsg:
            txt = QString("Info: %1").arg(msg);
            break;
        case QtFatalMsg:
            txt = QString("Fatal: %1").arg(msg);
            abort();
    }

    QFile outFile("log");
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << endl;
    QTextStream(stdout) << txt << endl;
}

void initAppInfo()
{
    QCoreApplication::setApplicationName("G-Pilot");
    QCoreApplication::setOrganizationName("BTS");
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(messageHandler);
    initAppInfo();

#ifdef UNIX
    bool styleOverrided = false;
    for (int i = 0; i < argc; i++) if (QString(argv[i]).toUpper() == "-STYLE") {
        styleOverrided = true;
        break;
    }
#endif

    QApplication app(argc, argv);
    app.setApplicationDisplayName("G-Pilot");
    app.setOrganizationName("G-Pilot");

#ifdef GLES
    QFontDatabase::addApplicationFont(":/fonts/Ubuntu-R.ttf");
#endif
    QGLFormat glf = QGLFormat::defaultFormat();
    glf.setSampleBuffers(true);
    glf.setSamples(8);
    QGLFormat::setDefaultFormat(glf);

    QSettings set(app.applicationDirPath() + "/settings.ini", QSettings::IniFormat);
    set.setIniCodec("UTF-8");
    QString loc = set.value("language", "en").toString();

    QString translationsFolder = qApp->applicationDirPath() + "/translations/";
    QString translationFileName = translationsFolder + "candle_" + loc + ".qm";

    if(QFile::exists(translationFileName)) {
        QTranslator* translator = new QTranslator();
        if (translator->load(translationFileName)) app.installTranslator(translator); else delete translator;
    }

    QString baseTranslationFileName = translationsFolder + "qt_" + loc + ".qm";

    if(QFile::exists(translationFileName)) {
        QTranslator* baseTranslator = new QTranslator();
        if (baseTranslator->load(baseTranslationFileName)) app.installTranslator(baseTranslator); else delete baseTranslator;
    }

#ifdef UNIX
    if (!styleOverrided) foreach (QString str, QStyleFactory::keys()) {
        qDebug() << "style" << str;
        if (str.contains("GTK+")) {
            a.setStyle(QStyleFactory::create(str));
            break;
        }
    }
#endif

#ifdef GLES
    a.setStyle(QStyleFactory::create("Fusion"));
    QPalette palette;
    palette.setColor(QPalette::Highlight, QColor(204, 204, 254));
    palette.setColor(QPalette::HighlightedText, QColor(0, 0, 0));
    a.setPalette(palette);

    a.setStyleSheet("QWidget {font-family: \"Ubuntu\";}\
                    QMenuBar {background-color: #303030; padding-top: 2px; padding-bottom: 2px;}\
                    QMenuBar::item {spacing: 3px; padding: 2px 8px; background: transparent; color: white;}\
                    QMenuBar::item:pressed {border: 1px solid #505050; border-bottom: 1px; border-top-left-radius: 3px; border-top-right-radius: 3px; background: #404040; color: white;}\
                    QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white;}\
                    QDialog {border: 1px solid palette(mid);}");
#endif

    app.setStyleSheet(app.styleSheet() + "QWidget {font-size: 8pt}");

    frmMain w;
    w.show();

    return app.exec();
}

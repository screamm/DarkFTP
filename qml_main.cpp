// qml_main.cpp - Separat huvudfil för QML-versionen
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QDebug>
#include <iostream>
#include <exception>
#include <QDir>
#include "src/filemodel.h" // Inkludera FileModel header

int main(int argc, char *argv[])
{
    try {
        // Aktivera high DPI-skalning
        QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
            Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

        QGuiApplication app(argc, argv);
        
        // Sätt Material-stil på kontroller
        QQuickStyle::setStyle("Material");
        
        // Skapa QML-motorn och ladda huvudfilen
        QQmlApplicationEngine engine;
        
        // Diagnostik för att hjälpa felsökning
        qDebug() << "Arbetskatalog:" << QDir::currentPath();
        qDebug() << "App sökväg:" << QCoreApplication::applicationDirPath();
        qDebug() << "QML importvägar:" << engine.importPathList();
        
        // Explicit ladda och registrera QML-komponenter
        engine.addImportPath("qrc:/");
        
        // Registrera FileModel för användning i QML
        qmlRegisterType<FileModel>("DarkFTP", 1, 0, "FileModel");

        // Skapa en instans av FileModel för lokala filer
        FileModel localFileModel(false); // false indikerar att det inte är en fjärrmodell
        // Skapa en instans för fjärrfiler (initialt tom eller inaktiv)
        FileModel remoteFileModel(true); // true indikerar att det är en fjärrmodell

        // Gör modellerna tillgängliga i QML-kontexten
        engine.rootContext()->setContextProperty("localFileModel", &localFileModel);
        engine.rootContext()->setContextProperty("remoteFileModel", &remoteFileModel);
        
        // Ladda QML-huvudfilen
        const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
        
        QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                         &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) {
                qDebug() << "Fel vid laddning av QML-fil:" << url;
                QCoreApplication::exit(-1);
            } else if (obj && url == objUrl) {
                qDebug() << "QML-fil laddad framgångsrikt:" << url;
            }
        }, Qt::QueuedConnection);
        
        engine.load(url);
        
        return app.exec();
    }
    catch (const std::exception& e) {
        qDebug() << "KRASCH VID START: " << e.what();
        std::cerr << "Ett allvarligt fel inträffade vid start: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        qDebug() << "OKÄND KRASCH VID START";
        std::cerr << "Ett okänt fel inträffade vid start av programmet." << std::endl;
        return 1;
    }
} 
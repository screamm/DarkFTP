// main.cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include <iostream>
#include <exception>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    try {
        QGuiApplication app(argc, argv);
        
        // Sätt Material-stil på kontroller
        QQuickStyle::setStyle("Material");
        
        // Skapa MainWindow-instans
        MainWindow mainWindow;
        
        // QML-motorn
        QQmlApplicationEngine engine;
        
        // Sökväg till byggkatalogen där QML-filerna finns
        QDir appDir(QCoreApplication::applicationDirPath());
        QString qmlPath = appDir.absolutePath() + "/qml";
        
        // Lägg till sökvägen till QML-importsökvägar
        engine.addImportPath(appDir.absolutePath());
        
        // Debug-utskrifter
        qDebug() << "Arbetskatalog:" << QDir::currentPath();
        qDebug() << "App sökväg:" << appDir.absolutePath();
        qDebug() << "QML importvägar:" << engine.importPathList();
        
        // Exponera MainWindow-instansen till QML
        engine.rootContext()->setContextProperty("backend", &mainWindow);
        
        // Ladda QML-huvudfilen
        const QUrl url(appDir.absolutePath() + "/qml/main.qml");
        qDebug() << "Laddar QML från:" << url.toString();
        
        QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                         &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) {
                qDebug() << "Misslyckades med att skapa QML-objekt";
                QCoreApplication::exit(-1);
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

// main.cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QDebug>
#include <iostream>
#include <exception>

int main(int argc, char *argv[])
{
    try {
        QGuiApplication app(argc, argv);
        
        // Sätt Material-stil på kontroller
        QQuickStyle::setStyle("Material");
        
        // Skapa QML-motorn och ladda huvudfilen
        QQmlApplicationEngine engine;
        
        // Registrera C++-objekt som ska vara tillgängliga i QML här
        // ex: qmlRegisterType<FtpManager>("FtpComponents", 1, 0, "FtpManager");
        
        // Ladda QML-huvudfilen
        const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
        
        QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                         &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
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

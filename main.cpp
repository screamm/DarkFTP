// main.cpp
#include <QApplication>
#include "mainwindow.h"
#include <QMessageBox>
#include <QDebug>
#include <iostream>
#include <exception>

int main(int argc, char *argv[])
{
    try {
        QApplication a(argc, argv);
        
        // Försök skapa och visa huvudfönstret
        try {
            MainWindow w;
            w.show();
            return a.exec();
        }
        catch (const std::exception& e) {
            qDebug() << "KRASCH VID START: " << e.what();
            QMessageBox::critical(nullptr, "Krasch vid start", 
                                 QString("Ett allvarligt fel inträffade vid start: %1").arg(e.what()));
            return 1;
        }
        catch (...) {
            qDebug() << "OKÄND KRASCH VID START";
            QMessageBox::critical(nullptr, "Krasch vid start", 
                                 "Ett okänt fel inträffade vid start av programmet.");
            return 1;
        }
    }
    catch (...) {
        std::cerr << "Allvarligt fel vid initialisering av applikationen!" << std::endl;
        return 1;
    }
}

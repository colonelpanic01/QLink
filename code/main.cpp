#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QFile>
#include <iostream>

#include "ui/MainWindow.h"
#include "core/common/QLinkException.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("Qlink");
    
    // Load modern stylesheet from file
    QFile styleFile(":/resources/styles/modern.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString styleSheet = QLatin1String(styleFile.readAll());
        app.setStyleSheet(styleSheet);
        styleFile.close();
        std::cout << "Stylesheet loaded successfully" << std::endl;
    } else {
        std::cerr << "Failed to load stylesheet: " << styleFile.errorString().toStdString() << std::endl;
        std::cerr << "Continuing without custom styling..." << std::endl;
    }
    
    // app.setOrganizationName("CS3307A");
    try {
        std::cout << "Creating main window..." << std::endl;
        // Create main window
        qlink::MainWindow window;
        std::cout << "Main window created, showing..." << std::endl;
        window.show();
        std::cout << "Starting event loop..." << std::endl;
        
        return app.exec();
        
    } catch (const qlink::QLinkException& e) {
        std::cerr << "Qlink Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected Error: " << e.what() << std::endl;
        return 1;
    }
}
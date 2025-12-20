#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <iostream>

#include "ui/MainWindow.h"
#include "core/common/QLinkException.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("Qlink");
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
/**
 * @file main.cpp
 * @brief Application Entry Point for LogReader
 * @details Contains the main function and application initialization logic.
 *          Sets up exception handling, initializes language management,
 *          and launches the main user interface.
 * @author GeziP
 * @date 2025-06-27
 * @version 1.0
 * @copyright MIT License
 */

#include <QApplication>
#include <QDebug>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QTranslator>
#include <exception>

#include "ui/logviewer.h"
#include "utils/languagemanager.h"

/**
 * @brief Custom terminate handler for unhandled exceptions
 * @details This function is called when an unhandled exception occurs.
 *          It displays an error message to the user and safely terminates
 *          the application to prevent crashes.
 */
void myTerminateHandler()
{
    qDebug() << "Unhandled exception caught!";
    QMessageBox::critical(nullptr, "错误", "程序遇到未处理的异常，即将退出。");
    std::abort();
}

/**
 * @brief Main application entry point
 * @param argc Number of command line arguments
 * @param argv Array of command line argument strings
 * @return Application exit code (0 for success, non-zero for error)
 * @details Initializes the Qt application, sets up exception handling,
 *          configures application metadata, initializes the language manager,
 *          and displays the main window.
 */
int main(int argc, char* argv[])
{
    // Set up custom exception handler for better error handling
    std::set_terminate(myTerminateHandler);

    // Create Qt application instance
    QApplication a(argc, argv);

    // Set application metadata for QSettings and about dialogs
    a.setApplicationName("LogReader");
    a.setApplicationVersion("1.0");
    a.setOrganizationName("LogViewer");
    a.setOrganizationDomain("logviewer.org");

    // Debug output for troubleshooting
    qDebug() << "Application starting...";
    qDebug() << "Application directory:" << QApplication::applicationDirPath();

    // Initialize language management system
    qDebug() << "Initializing language manager...";
    LanguageManager::instance().initialize();

    // Create and show main window
    LogViewer w;
    w.show();

    // Start Qt event loop
    return a.exec();
}

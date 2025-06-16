#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QDebug>
#include <exception>
#include <logviewer.h>

void myTerminateHandler()
{
    qDebug() << "Unhandled exception caught!";
    QMessageBox::critical(nullptr, "错误", "程序遇到未处理的异常，即将退出。");
    std::abort();
}

int main(int argc, char *argv[])
{
    std::set_terminate(myTerminateHandler);
    QApplication a(argc, argv);

    QStringList candidates = {
        QLocale::system().name(),
        QLocale::system().languageToString(QLocale::system().language()).toLower(),
        "en_US",
        "en"
    };

    QTranslator qtTranslator;
    for (const QString& localeName : candidates)
    {
        if (qtTranslator.load("translation_" + localeName, QCoreApplication::applicationDirPath()))
            break;
    }
    a.installTranslator(&qtTranslator);

    LogViewer w;
    w.show();

    return a.exec();
}

#include "mainwindow.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("SimpleTextEditor"));
    app.setApplicationDisplayName(QStringLiteral("简单文本编辑器"));
    app.setApplicationVersion(QStringLiteral("1.0.0"));
    app.setOrganizationName(QStringLiteral("DGUT"));
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/icons/app.svg")));

    MainWindow window;
    window.show();

    return app.exec();
}

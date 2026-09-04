#include <QApplication>
#include <QPalette>
#include <QStyleFactory>

#include "ApiClient.h"
#include "MainWindow.h"

static void applyDarkPalette(QApplication& app)
{
    app.setStyle(QStyleFactory::create("Fusion"));

    QPalette palette;
    palette.setColor(QPalette::Window, QColor(37, 37, 38));
    palette.setColor(QPalette::WindowText, QColor(220, 220, 220));
    palette.setColor(QPalette::Base, QColor(30, 30, 30));
    palette.setColor(QPalette::AlternateBase, QColor(45, 45, 48));
    palette.setColor(QPalette::Text, QColor(220, 220, 220));
    palette.setColor(QPalette::Button, QColor(60, 60, 60));
    palette.setColor(QPalette::ButtonText, QColor(220, 220, 220));
    palette.setColor(QPalette::Highlight, QColor(0, 120, 215));
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::PlaceholderText, QColor(140, 140, 140));

    app.setPalette(palette);
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    applyDarkPalette(app);

    ApiClient apiClient;
    MainWindow window(&apiClient);
    window.show();

    return app.exec();
}

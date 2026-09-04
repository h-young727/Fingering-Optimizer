#pragma once

#include <QMainWindow>

class ApiClient;
class LibraryView;
class LoginView;
class QStackedWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(ApiClient* apiClient, QWidget* parent = nullptr);

private:
    QStackedWidget* stack;
    LoginView* loginView;
    LibraryView* libraryView;
};

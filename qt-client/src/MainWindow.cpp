#include "MainWindow.h"

#include <QStackedWidget>

#include "ApiClient.h"
#include "LibraryView.h"
#include "LoginView.h"

MainWindow::MainWindow(ApiClient* apiClient, QWidget* parent)
    : QMainWindow(parent)
{
    stack = new QStackedWidget(this);
    loginView = new LoginView(apiClient, this);
    libraryView = new LibraryView(apiClient, this);

    stack->addWidget(loginView);
    stack->addWidget(libraryView);
    stack->setCurrentWidget(loginView);

    setCentralWidget(stack);
    setWindowTitle("Fingering Optimizer");
    resize(800, 500);

    connect(loginView, &LoginView::loginSucceeded, this, [this]()
    {
        stack->setCurrentWidget(libraryView);
        libraryView->refresh();
    });

    connect(apiClient, &ApiClient::loggedOut, this, [this]()
    {
        loginView->clearFields();
        stack->setCurrentWidget(loginView);
    });
}

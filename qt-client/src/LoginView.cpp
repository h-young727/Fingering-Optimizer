#include "LoginView.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "ApiClient.h"

LoginView::LoginView(ApiClient* apiClient, QWidget* parent)
    : QWidget(parent)
    , apiClient(apiClient)
{
    emailField = new QLineEdit(this);
    emailField->setPlaceholderText("Email");

    passwordField = new QLineEdit(this);
    passwordField->setPlaceholderText("Password");
    passwordField->setEchoMode(QLineEdit::Password);

    statusLabel = new QLabel(this);

    QPushButton* loginButton = new QPushButton("Log In", this);
    QPushButton* signupButton = new QPushButton("Sign Up", this);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addStretch();
    layout->addWidget(emailField);
    layout->addWidget(passwordField);
    layout->addWidget(loginButton);
    layout->addWidget(signupButton);
    layout->addWidget(statusLabel);
    layout->addStretch();

    connect(loginButton, &QPushButton::clicked, this, [this]()
    {
        statusLabel->setText("Logging in...");
        this->apiClient->login(emailField->text(), passwordField->text());
    });

    connect(signupButton, &QPushButton::clicked, this, [this]()
    {
        statusLabel->setText("Signing up...");
        this->apiClient->signup(emailField->text(), passwordField->text());
    });

    connect(apiClient, &ApiClient::loginSucceeded, this, [this]()
    {
        statusLabel->setText("");
        emit loginSucceeded();
    });

    connect(apiClient, &ApiClient::loginFailed, this, [this](const QString& message)
    {
        statusLabel->setText(message);
    });

    connect(apiClient, &ApiClient::signupSucceeded, this, [this]()
    {
        statusLabel->setText("Account created, logging in...");
        this->apiClient->login(emailField->text(), passwordField->text());
    });

    connect(apiClient, &ApiClient::signupFailed, this, [this](const QString& message)
    {
        statusLabel->setText(message);
    });
}

void LoginView::clearFields()
{
    emailField->clear();
    passwordField->clear();
    statusLabel->setText("");
}

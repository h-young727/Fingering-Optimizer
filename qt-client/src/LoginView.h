#pragma once

#include <QWidget>

class ApiClient;
class QLabel;
class QLineEdit;

class LoginView : public QWidget
{
    Q_OBJECT

public:
    explicit LoginView(ApiClient* apiClient, QWidget* parent = nullptr);

    void clearFields();

signals:
    void loginSucceeded();

private:
    ApiClient* apiClient;
    QLineEdit* emailField;
    QLineEdit* passwordField;
    QLabel* statusLabel;
};

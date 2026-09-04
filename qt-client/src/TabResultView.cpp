#include "TabResultView.h"

#include <QFont>
#include <QPlainTextEdit>
#include <QTabWidget>
#include <QVBoxLayout>

TabResultView::TabResultView(QWidget* parent)
    : QWidget(parent)
{
    tabWidget = new QTabWidget(this);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(tabWidget);
}

void TabResultView::showResults(const QList<LabeledTab>& tabs)
{
    tabWidget->clear();

    QFont monospaceFont("Monospace");
    monospaceFont.setStyleHint(QFont::TypeWriter);

    for (const LabeledTab& tab : tabs)
    {
        QPlainTextEdit* textEdit = new QPlainTextEdit(tabWidget);
        textEdit->setReadOnly(true);
        textEdit->setFont(monospaceFont);
        textEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
        textEdit->setPlainText(tab.tabText);

        tabWidget->addTab(textEdit, tab.label);
    }
}

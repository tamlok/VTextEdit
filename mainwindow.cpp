#include "mainwindow.h"

#include <QtWidgets>

#include "vtextdocumentlayout.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUI();

    writeTestText();
}

MainWindow::~MainWindow()
{

}

void MainWindow::setupUI()
{
    m_edit = new QTextEdit();
    VTextDocumentLayout *docLayout = new VTextDocumentLayout(m_edit->document());
    m_edit->document()->setDocumentLayout(docLayout);

    setCentralWidget(m_edit);
}

void MainWindow::writeTestText()
{
    QTextCursor cursor = m_edit->textCursor();

    cursor.insertText("Text");

    cursor.insertBlock();

    QTextCharFormat fmt = cursor.charFormat();
    fmt.setForeground(QColor("red"));

    cursor.setCharFormat(fmt);

    cursor.insertText("Test a layout.");

    QString text = "1234567890\n"
                   "2234567890\n"
                   "3234567890\n"
                   "4234567890\n"
                   "5234567890\n"
                   "6234567890\n"
                   "7234567890\n"
                   "8234567890\n"
                   "9234567890\n";

    cursor.insertText(text);
}

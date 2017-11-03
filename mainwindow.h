#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class VTextEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void setupUI();

    void writeTestText();

    VTextEdit *m_edit;
};

#endif // MAINWINDOW_H

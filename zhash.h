#ifndef ZHASH_H
#define ZHASH_H

#include <QMainWindow>

namespace Ui {
    class zHash;
}

class zHash : public QMainWindow
{
    Q_OBJECT

public:
    explicit zHash(QWidget *parent = 0);
    ~zHash();
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private:
    Ui::zHash *ui;
    void setupConnections();
    QByteArray md5res, sha1res;
    QString filePath;

private slots:
    void loadFile();
    void verifyVHF();
    void verifyMD5();
    void verifySHA1();
    void saveFile();
    void about();
    void aboutQT();
    void inputString();
};

#endif // ZHASH_H

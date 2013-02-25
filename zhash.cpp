#include "zhash.h"
#include "ui_zhash.h"
#include <QtGui>
#include <QCryptographicHash>
#include <QTextStream>
#include <QPalette>
#include <QString>

zHash::zHash(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::zHash)
{
    ui->setupUi(this);
    setAcceptDrops(true);
    ui->md5Edit->setAcceptDrops(false);
    ui->sha1Edit->setAcceptDrops(false);
    setWindowTitle("ZaeSoft zHash");
    setFixedSize(375,146); //We do not want the user to be able to
    ui->statusBar->setSizeGripEnabled(false); //Resize the window.
    setupConnections(); //Sets up the connections
}

void zHash::setupConnections()
{//Connects the menu actions to functions
    connect(ui->actionOpen,SIGNAL(triggered(bool)), this, SLOT(loadFile()));
    connect(ui->actionInput_String, SIGNAL(triggered(bool)), this, SLOT(inputString()));
    connect(ui->actionSave, SIGNAL(triggered(bool)), this, SLOT(saveFile()));
    connect(ui->actionExit, SIGNAL(triggered(bool)), this, SLOT(close()));

    connect(ui->actionVerify_VHF, SIGNAL(triggered(bool)), this, SLOT(verifyVHF()));
    connect(ui->actionVerify_MD5, SIGNAL(triggered(bool)), this, SLOT(verifyMD5()));
    connect(ui->actionVerify_SHA_1, SIGNAL(triggered(bool)), this, SLOT(verifySHA1()));

    connect(ui->actionAbout_zHash, SIGNAL(triggered(bool)), this, SLOT(about()));
    connect(ui->actionAbout_QT, SIGNAL(triggered(bool)), this, SLOT(aboutQT()));
}

//START File Menu
void zHash::loadFile()
{//Loads a file and generates MD5 and SHA-1 information for that file
    QCryptographicHash md5(QCryptographicHash::Md5);//The MD5 hash
    QCryptographicHash sha1(QCryptographicHash::Sha1);//The SHA1 hash
    QString filename = QFileDialog::getOpenFileName(this, "Open File", "","All Files (*.*)"); //Gets the name of the file to open
    if(filename.isEmpty())
        return; //We dont need to continue if the user closed the dialog.

    QFileInfo pathInfo(filename); //For showing the filename
    QFile file(filename);

    if(!file.open(QIODevice::ReadOnly)) //Opens the file in ReadOnly mode
    {
        QMessageBox::critical(this, "File could not be opened!", "The File \"" + pathInfo.fileName() + "\" could not be opened in Read-Only Mode! Open aborted!");
        return; //If it doesnt work, stop progression
    }

    filePath = filename; //Sets the QString filePath variable
    while (!file.atEnd())
    {
        QByteArray temp = file.read(4096); //Read the file in 4MB blocks to avoid excess memory usage
        md5.addData(temp);  //Add the data to the md5 hash
        sha1.addData(temp); //Add the data to the SHA-1 hash
        temp.clear();
    }

    file.close(); //Close the file

    md5res.clear(); //Clears any previous data in QByteArray MD5res
    sha1res.clear();//Clears any previous data in QByteArray sha1res

    md5res = md5.result(); //Sets QByteArray md5res to the result of the MD5sum of the file
    sha1res = sha1.result(); //Sets QByteArray sha1res to the result of the SHA1sum of the file

    ui->md5Edit->setText(md5res.toHex()); //Put the hex in md5Edit
    ui->sha1Edit->setText(sha1res.toHex()); //Put the hex in sha1Edit
    ui->fileLabel->setText("<b>" + pathInfo.fileName() + "</b>"); //Sets the file label in bold
    setWindowTitle(pathInfo.fileName() + " - Zaesoft zHash");
    ui->statusBar->showMessage(pathInfo.fileName() + " loaded successfully!",1500);
    ui->actionSave->setEnabled(true);         //Enable the greyed out
    ui->actionVerify_MD5->setEnabled(true);   //menu options because
    ui->actionVerify_SHA_1->setEnabled(true); //a file has been selected
    ui->actionVerify_VHF->setEnabled(true);   //and now the options are usable
}

void zHash::inputString()
{
    QCryptographicHash md5(QCryptographicHash::Md5);//The MD5 hash
    QCryptographicHash sha1(QCryptographicHash::Sha1);//The SHA1 hash
    bool ok;
    QString input = QInputDialog::getText(this, "Input A String",
                                          "Please enter a string to be hashed",
                                          QLineEdit::Normal, "", &ok);
    if(ok)
    {
        md5.addData(input.toAscii());
        sha1.addData(input.toAscii());

        md5res = md5.result();
        sha1res = sha1.result();

        ui->md5Edit->setText(md5res.toHex());
        ui->sha1Edit->setText(sha1res.toHex());
        ui->fileLabel->setText("<b>String Input</b>");
        setWindowTitle("Hashes for String Input - ZaeSoft zHash");

        ui->actionSave->setDisabled(true);         //Enable the greyed out
        ui->actionVerify_MD5->setDisabled(false);   //menu options because
        ui->actionVerify_SHA_1->setDisabled(false); //a file has been selected
        ui->actionVerify_VHF->setDisabled(true);   //and now the options are usable
    }
}

void zHash::saveFile()
{//Saves a file with the hash information and filename
    QString filename;
    filename = QFileDialog::getSaveFileName(this, "Save zHash File", "", "zHash Verifier Format (*.zhf);;Plain Text File (*.txt)");
    if(filename.isEmpty())
        return; //Empty filenames are no good for saving!

    QFile file(filename); //File for interfacing Output
    QFileInfo pathInfo(filename); //Gives path information for the saved file
    QFileInfo hasherInfo(filePath); //Gives path information for the hashed file

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) //Opening the device for writing text
    {
        QMessageBox::critical(this, "Could not open device for writing", "zHash could not open the device for writing. Write aborted.");
        return; //That didnt work, lets get out of this function
    }

    QTextStream out(&file); //We'll use a QTextStream to write the 3 lines of data, formatted logically
    out << "MD5: " << md5res.toHex() << "\nSHA-1: " << sha1res.toHex() << "\nFilename: " << hasherInfo.fileName();

    file.close(); //Closes the file
    ui->statusBar->showMessage(pathInfo.fileName() + " Saved Successfully!", 1500);
}
//END File Menu

//START Verifier Menu
void zHash::verifyVHF()
{//Verifies the Hash using the data in a VHF file.
    QString filename = QFileDialog::getOpenFileName(this, "Open Hash File", "","zHash Verifier File (*.zhf);; Text File (*.txt)"); //Gets the name of the file to open
    if(filename.isEmpty())
        return; //No need to continue without a valid VHF file

    QFileInfo pathInfo(filename); //Path information about the VHF file
    QFileInfo hasherInfo(filePath); //Path information for the hashed file
    QFile file(filename); //File for interfacing

    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, "ERROR! File could not be opened for reading!", "The file \""
                              + pathInfo.fileName() + "\" could NOT be opened for reading. Open Aborted.");
        return;
    }

    QByteArray md5sum = file.readLine(); //Reads line 1, which contains MD5 information
    QByteArray sha1sum = file.readLine(); //Reads line 2, which contains SHA1 information
    file.close(); //We've got what we need.

    if(!md5sum.startsWith("MD5: ") || !sha1sum.startsWith("SHA-1: "))
    {//Checks the format, if it is wrong, we will return and spit an error
        QMessageBox::critical(this, "Wrong VHF Format!", "The file \"" + pathInfo.fileName() +
                              "\" is not in the proper format. Sums cannot be verified.");
        return;
    }

    QString md5Str(md5sum.remove(0,5)); //Remove the identification bytes
    QString sha1Str(sha1sum.remove(0,7)); //Remove the identification bytes

    md5Str.truncate(32);  //Remove any newlines or carriage returns
    sha1Str.truncate(40); //Remove any newlines or carriage returns

    if(md5Str == md5res.toHex() && sha1Str == sha1res.toHex())
    {//DO the hashes match?
        QMessageBox::information(this, hasherInfo.fileName() + " Verified!", "The hashes stored in \"" + pathInfo.fileName() + "\" matched those of the file \"" + hasherInfo.fileName() + "\".\nCongratulations, the file is valid!");
        ui->statusBar->showMessage("VHF Verified.",3000);
    }
    else //Well they didnt match, let the user know gently. Only a warning, not a critical
        QMessageBox::warning(this, "Hashes did not match!", "The MD5 and SHA-1 hashes stored in \"" + pathInfo.fileName() + "\" did NOT match the hashes of \"" + hasherInfo.fileName() + "\".\n The file is NOT valid for these hashes.");
}
void zHash::verifyMD5()
{//Checks a user-provided MD5Sum against the generated one
    QFileInfo pathInfo(filePath); //file information for the hashed file
    bool ok; //Did the user hit ok? if not, do nothing.
    QString result = QInputDialog::getText(this, "Verify MD5 checksum","Enter MD5 checksum", QLineEdit::Normal, "", &ok);
    if(ok && result == md5res.toHex())
    {//We've got a match!
        QMessageBox::information(this, "MD5 Checksum match!", "The MD5 checksum provided and the MD5 checksum for \"" + pathInfo.fileName() + "\" have been successfully matched. The file is valid.");
        ui->statusBar->showMessage("MD5 Verified.", 3000);
    }
    else if(ok) //No match.
        QMessageBox::warning(this, "MD5 Checksums did NOT match!", "The MD5 checksum provided and the MD5 checksum for \"" + pathInfo.fileName() + "\" were NOT successfully matched. Checksums are not equal. The file is not valid for the hash.");
}
void zHash::verifySHA1()
{//Checks a user-provided SHA-1Sum agains the generatoed one.
    QFileInfo pathInfo(filePath);
    bool ok; //did the user hit ok?
    QString result = QInputDialog::getText(this, "Verify SHA-1 checksum","Enter SHA-1 checksum", QLineEdit::Normal, "", &ok);
    if(ok && result == sha1res.toHex())
    {//We've got a match!
        QMessageBox::information(this, "SHA-1 Checksum match!", "The SHA-1 checksum provided and the SHA-1 checksum for " + pathInfo.fileName() + " have been successfully matched. The file is valid.");
        ui->statusBar->showMessage("SHA-1 Verified.", 3000);
    }
    else if(ok) //No match.
        QMessageBox::warning(this, "SHA-1 Checksums did NOT match!", "The SHA-1 checksum provided and the SHA-1 checksum for " + pathInfo.fileName() + " were NOT successfully matched. Checksums are not equal. The file is not valid for the hash.");

}
//END Verifier Menu

//START About Menu
void zHash::about()
{//Displays information about zHash
    QMessageBox::about(this, "About zHash",
                       "<p>zHash is a simple program by Mike Nawrocki to calculate and verify "
                       "MD5 and SHA-1 checksums.</p> "
                       "<p>Icon Theme by Carpelinx at KDE-look.org. Application Icon by "
                       " Maywolf @ Deviantart.</p> "
                       "<p>Licensed under the GPL, open-source. Feel free to modify and distribute.</p>"
                       "<p>zHash Version 1.0.1.2");
}
void zHash::aboutQT()
{//Displays information about QT
    QMessageBox::aboutQt(this, "About QT");
}
//END about menu

//Start Drag&Drop Handlers
void zHash::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}
void zHash::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return;

    QCryptographicHash md5(QCryptographicHash::Md5);//The MD5 hash
    QCryptographicHash sha1(QCryptographicHash::Sha1);//The SHA1 hash
    QString filename = urls.first().toLocalFile();
    if(filename.isEmpty())
        return; //We dont need to continue if the user closed the dialog.

    QFileInfo pathInfo(filename); //For showing the filename
    QFile file(filename);

    if(!file.open(QIODevice::ReadOnly)) //Opens the file in ReadOnly mode
    {
        QMessageBox::critical(this, "File could not be opened!", "The File \"" + pathInfo.fileName() + "\" could not be opened in Read-Only Mode! Open aborted!");
        return; //If it doesnt work, stop progression
    }

    filePath = filename; //Sets the QString filePath variable
    while (!file.atEnd())
    {
        QByteArray temp = file.read(4096*1024); //Read the file in 4MB blocks to avoid excess memory usage
        md5.addData(temp);  //Add the data to the md5 hash
        sha1.addData(temp); //Add the data to the SHA-1 hash
        temp.clear();
    }

    file.close(); //Close the file

    md5res.clear(); //Clears any previous data in QByteArray MD5res
    sha1res.clear();//Clears any previous data in QByteArray sha1res

    md5res = md5.result(); //Sets QByteArray md5res to the result of the MD5sum of the file
    sha1res = sha1.result(); //Sets QByteArray sha1res to the result of the SHA1sum of the file

    ui->md5Edit->setText(md5res.toHex()); //Put the hex in md5Edit
    ui->sha1Edit->setText(sha1res.toHex()); //Put the hex in sha1Edit
    ui->fileLabel->setText("<b>" + pathInfo.fileName() + "</b>"); //Sets the file label in bold
    setWindowTitle(pathInfo.fileName() + " - Zaesoft zHash");
    ui->statusBar->showMessage(pathInfo.fileName() + " loaded successfully!",1500);
    ui->actionSave->setEnabled(true);         //Enable the greyed out
    ui->actionVerify_MD5->setEnabled(true);   //menu options because
    ui->actionVerify_SHA_1->setEnabled(true); //a file has been selected
    ui->actionVerify_VHF->setEnabled(true);   //and now the options are usable
    }

//End Drag&Drop Handlers

zHash::~zHash()
{
    delete ui;
}


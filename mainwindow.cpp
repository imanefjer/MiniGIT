// mainwindow.cpp
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "miniversioncontrol.h"

#include <QDir>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), vcs(new MiniVersionControl) {
    ui->setupUi(this);
    ui->terminal->setReadOnly(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_initButton_clicked()
{
    this->logUserAction(QString("Initiating your Repository..."));
    //QMessageBox::information(this, "Add Files", "Selected Files:\n" + selectedFiles.join("\n"));
    // Executing Function Logic ...
    // ...
    MainWindow::vcs->init();
    this->logUserAction(QString("Initiation Stage Completed."));
}


// Handlers for special buttons
void MainWindow::on_addButton_clicked()
{
    this->logUserAction("Adding Selected Elements...");

    for (const QString& selectedFile : this->selectedFiles)
    {
        std::string filePath = selectedFile.toStdString();
        this->vcs->add(filePath);

    }

    this->logUserAction("Adding Stage Completed.");
    MainWindow::on_pushButton_pressed();
    MainWindow::on_pushButton_2_clicked();

}









// Done.
void MainWindow::on_commitButton_clicked()
{

    this->logUserAction(QString("Your Changes are Being Commited..."));
    //QMessageBox::information(this, "Add Files", "Selected Files:\n" + selectedFiles.join("\n"));

    this->vcs->commit("Mahmoud/Amine/Imane");

    this->logUserAction(QString("Your Changes Have succesfully been commited."));

    MainWindow::on_pushButton_pressed();
    MainWindow::on_pushButton_2_clicked();
    // MainWindow::on_reloadVersions_pressed();
}



//
void MainWindow::on_pushButton_pressed()
{
    ui->filesList->clear();

    std::vector<std::string> vec =  this->vcs->listFilesAndFolders();

    for(auto x : vec){
        QString itemText =  QString::fromStdString(x); // Example text: "Item 1", "Item 2", ...
        QListWidgetItem *newItem = new QListWidgetItem(itemText);

        // You can set additional properties or customize the item here if needed

        ui->filesList->addItem(newItem);
    }


}


void MainWindow::on_reloadVersions_pressed()
{
    ui->versionsList->clear();

    std::vector<std::string> vec =  this->vcs->listVersions();
    int i = 1;
    for(auto x : vec){
        std::string res = x;
        res = res + " - Version: " + std::to_string(i);
        QString itemText =  QString::fromStdString(res); // Example text: "Item 1", "Item 2", ...
        QListWidgetItem *newItem = new QListWidgetItem(itemText);

        ui->versionsList->addItem(newItem);
        i++;
    }
}



// This function changes the selected Version.
void MainWindow::on_versionsList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{

    // Set some attribute (i.e currVerison : To the version id)
    // QString s = current->text();

    current->setBackground(QBrush(Qt::yellow));
    isSelectedVersion = true;
    version = current->text();


    // Set The visuals for the previous element.
    if(previous != NULL){
        previous->setBackground(QBrush(Qt::white));
    }
}


// This function adds all items in the filesList to the selectedFiles structure.
void MainWindow::on_selectAll_clicked()
{
    QListWidget *filesList = ui->filesList;
    this->selectedFiles.clear();


    for (int i = 0; i < filesList->count(); ++i)
    {
        QListWidgetItem *item = filesList->item(i);
        if (item != nullptr)
        {
            // Add it to the list (it's name).
            this->selectedFiles.insert(item->text());
            item->setBackground(QBrush(Qt::yellow));
        }
    }

}


// This method Handles adding Selected files to the selectedFiles QSet<QString> object.
//====================================================
void MainWindow::on_filesList_itemClicked(QListWidgetItem *item)
{

    // this->logUserAction( QString("Clicked."));

    if(item->background() == QBrush(Qt::yellow)){
        // Remove the coloring.
        // remove the element from the list of elements to be commited.
        this->selectedFiles.remove(item->text());
        item->setBackground(QBrush(Qt::white));

    }else{
        // Add the coloring.
        this->selectedFiles.insert(item->text());
        item->setBackground(QBrush(Qt::yellow));
        // append it to the commit list.
    }

    //item->setBackground(QBrush(Qt::yellow));
}


// This function logs what user does in the run section.
void MainWindow::logUserAction(const QString &action)
{
    // Get the current date and time
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString timestamp = currentDateTime.toString("> yyyy-MM-dd hh:mm:ss ");

    // Append the user action to the log
    ui->terminal->appendPlainText(timestamp + " - " + action);
}


void MainWindow::on_revert_clicked()
{
    this->logUserAction("Reverting to selected Version...");

    if(this->isSelectedVersion){
        this->vcs->revert(".git/commits/" + this->version.toStdString().substr(0, 10));
    }
    else{
        this->logUserAction("No Version chosen.");
        return;
    }

    this->logUserAction("Completed.");

}


void MainWindow::on_addedFiles_itemClicked(QListWidgetItem *item)
{

    //this->logUserAction( QString("Clicked."));

    if(item->background() == QBrush(Qt::yellow)){
        // Remove the coloring.
        // remove the element from the list of elements to be commited.
        this->selectedFilesAdd.remove(item->text());
        item->setBackground(QBrush(Qt::white));

    }else{
        // Add the coloring.
        this->selectedFilesAdd.insert(item->text());
        item->setBackground(QBrush(Qt::yellow));
        // append it to the commit list.
    }

    //item->setBackground(QBrush(Qt::yellow));

}




void MainWindow::on_pushButton_2_clicked()
{
    ui->addedFiles->clear();

    std::vector<std::string> vec =  this->vcs->listStagingArea();

    //this->logUserAction("vsvwxvbwfgw");

    for(auto x : vec){
        QString itemText =  QString::fromStdString(x); // Example text: "Item 1", "Item 2", ...
        QListWidgetItem *newItem = new QListWidgetItem(itemText);

        // You can set additional properties or customize the item here if needed

        ui->addedFiles->addItem(newItem);
    }
}


void MainWindow::on_deleteButton_pressed()
{
    ui->addedFiles->clear();

    QSet<QString> vec =  this->selectedFilesAdd;
    for(auto x : vec){
        std::string filename = x.toStdString();
        this->vcs->deleteFromStaging(filename);
    }


    MainWindow::on_pushButton_2_clicked();
}






void MainWindow::on_pushButton_clicked()
{

}


void MainWindow::on_commitButton_pressed()
{

}



void MainWindow::on_versionsList_itemClicked(QListWidgetItem *item)
{

}


void MainWindow::on_versionsList_itemSelectionChanged()
{

}


void MainWindow::on_versionsList_itemPressed(QListWidgetItem *item)
{

}


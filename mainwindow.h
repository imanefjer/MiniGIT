// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <miniversioncontrol.h>

#include <QMainWindow>
#include <QListWidgetItem>
#include <QSet>



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    //void On_addButton_clicked();
    //void On_initButton_clicked();
    //void On_commitButton_clicked();
    //void On_deleteButton_clicked();
    //void On_filesList_itemSelectionChanged();
    //void On_versionsList_itemSelectionChanged();

    void on_addButton_clicked();
    void on_initButton_clicked();
    void on_commitButton_clicked();
    void on_pushButton_pressed();
    void on_reloadVersions_pressed();
    void on_versionsList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_selectAll_clicked();
    void on_filesList_itemClicked(QListWidgetItem *item);
    void logUserAction(const QString &action);

    void on_revert_clicked();

    void on_addedFiles_itemClicked(QListWidgetItem *item);

    void on_pushButton_2_clicked();


    void on_deleteButton_pressed();


    void on_pushButton_clicked();

    void on_commitButton_pressed();


    void on_versionsList_itemClicked(QListWidgetItem *item);

    void on_versionsList_itemSelectionChanged();

    void on_versionsList_itemPressed(QListWidgetItem *item);

private:

    // The UI object.
    Ui::MainWindow *ui;

    // List of file names that are selected (to be added for next Commit/ Or to be deleted).
    QSet<QString> selectedFiles;
    QSet<QString> selectedFilesAdd;


    bool isSelectedVersion = false;
    QString version;


    MiniVersionControl *vcs;

    // MiniGit mn;


    void updateFileList();
    void updateVersionList();
};

#endif // MAINWINDOW_H

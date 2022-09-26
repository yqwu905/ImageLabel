#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QMainWindow>
#include <QObject>
#include <QPixmap>
#include <QPushButton>
#include <QSettings>
#include <QShortcut>
#include <QSizePolicy>
#include <QString>
#include <QStringList>
#include <iostream>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  QDir dir = QDir::home();
  QStringList get_img_filename_in_dir(QDir dir);
  int img_idx = 0;
  QStringList imgs = get_img_filename_in_dir(dir);
  QStringList labels;
  QDir dest;
  void move_img_to(QString src, QString label);
  void select_dir();
  void update_dir_label();
  void load_img_to_label();
  void load_config();
  void add_label_button(QString label, QString shortcut);
  void remove_all_button();
  void reload_button();
  void rm_img(QString src);
  void prev_img();
  void next_img();
  void set_labels(int idx, QString label);
  ~MainWindow();

private slots:
  void handleButton(QString shortcut);

private:
  Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H

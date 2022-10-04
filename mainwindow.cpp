#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <iostream>

using namespace std::string_view_literals;

QTextBrowser *MainWindow::s_textBrowser = 0;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  this->update_dir_button();
  this->load_img_to_label();
  this->labels = QStringList();
  QObject::connect(ui->dir_button, &QPushButton::released, this,
                   [this] { this->select_dir(); });
  QCoreApplication::setOrganizationName("pingline");
  QCoreApplication::setOrganizationDomain("yqwu.site");
  QCoreApplication::setApplicationName("Image Label");
  s_textBrowser = new QTextBrowser;
  ui->gridLayout->layout()->addWidget(s_textBrowser);
  QObject::connect(ui->actionEdit_setting, &QAction::triggered, this,
                   &MainWindow::edit_setting);
  QObject::connect(ui->actionRestore_default_setting, &QAction::triggered, this,
                   &MainWindow::restore_default_setting);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::add_label_button(QString label, QString shortcut) {
  QString button_text = QString("%1\n(%2)").arg(label, shortcut);
  QPushButton *button = new QPushButton(button_text);
  button->setVisible(true);
  button->setSizePolicy(QSizePolicy::MinimumExpanding,
                        QSizePolicy::MinimumExpanding);
  ui->horizontalLayout->layout()->addWidget(button);
  QShortcut *shortcut_slot = new QShortcut(QKeySequence(shortcut), this);
  QObject::connect(shortcut_slot, SIGNAL(activated()), button,
                   SLOT(animateClick()));
  QObject::connect(button, &QPushButton::released, this,
                   [this, shortcut] { handleButton(shortcut); });
}

void MainWindow::handleButton(QString shortcut) {
  QSettings settings;
  qDebug() << "Button " << shortcut << " pressed.";
  if (settings.value("Prev").toString() == shortcut) {
    qDebug("Map to Prev.");
    this->prev_img();
  } else if (settings.value("Next").toString() == shortcut) {
    qDebug("Map to Next.");
    this->next_img();
  } else if (settings.value("Delete").toString() == shortcut) {
    qDebug("Map to Delete.");
    this->rm_img(this->imgs.at(this->img_idx));
  } else
    for (QString label : settings.childGroups()) {
      settings.beginGroup(label);
      if (settings.value("shortcut").toString() == shortcut) {
        qDebug() << "Map to " << label;
        this->move_img_to(this->imgs.at(this->img_idx), label);
        break;
      }
      settings.endGroup();
    }
}

QStringList MainWindow::get_img_filename_in_dir(QDir dir) {
  QStringList imgs = dir.entryList(QStringList() << "*.jpg"
                                                 << "*.png"
                                                 << "*.JPG"
                                                 << "*.PNG"
                                                 << "*.webp",
                                   QDir::Files | QDir::Readable);
  return imgs;
}

void MainWindow::select_dir() {
  QString selected_dir = QFileDialog::getExistingDirectory(
      this, tr("Select directory"), this->dir.absolutePath(),
      QFileDialog::ShowDirsOnly);
  if (!selected_dir.isEmpty()) {
    qDebug() << "Change directory from " << this->dir.absolutePath() << " to "
             << selected_dir;
    this->dir.setPath(selected_dir);
    this->update_dir_button();
    this->imgs = get_img_filename_in_dir(this->dir);
    this->img_idx = 0;
    this->labels = QStringList();
    load_img_to_label();
  } else {
    qDebug() << "File dialog canceled, directory not change.";
  }
  QSettings settings;
  this->dest.setPath(QDir::cleanPath(
      this->dir.filePath(settings.value("BaseDir").toString())));
}

void MainWindow::update_dir_button() {
  ui->dir_button->setText(QString("%1").arg(this->dir.absolutePath()));
}

void MainWindow::load_img_to_label() {
  if (this->img_idx >= this->imgs.size()) {
    ui->label->setText("All done!");
  } else {
    QString img_path = this->dir.filePath(imgs.at(this->img_idx));
    qDebug() << "Load image " << this->img_idx << ":" << img_path;
    QPixmap pixmap(img_path);
    ui->label->setPixmap(pixmap);
  }
}

void create_setting_label(QString label, QString shortcut, QString dir) {
  QSettings s;
  s.beginGroup(label);
  s.setValue("shortcut", shortcut);
  s.setValue("dir", dir);
  s.endGroup();
}

void MainWindow::load_config() {
  QSettings settings;
  if (!settings.contains("BaseDir")) {
    settings.clear();
    settings.setValue("BaseDir", "./");
    settings.setValue("Prev", "p");
    settings.setValue("Next", "n");
    settings.setValue("Delete", "d");
    create_setting_label("Lyy", "l", "lyy");
    create_setting_label("Screenshot", "s", "屏幕快照");
    create_setting_label("ACG", "a", "二刺螈");
    create_setting_label("Meme", "m", "meme");
    create_setting_label("Image Macro", "i", "表情包");
    create_setting_label("Camera", "c", "相机");
  }
  this->dest.setPath(QDir::cleanPath(
      this->dir.filePath(settings.value("BaseDir").toString())));
  qDebug() << "Setting file:" << settings.fileName();
}

void MainWindow::remove_all_button() {
  while (QLayoutItem *item = ui->horizontalLayout->takeAt(0)) {
    delete item->widget();
    delete item;
  }
}

void MainWindow::reload_button() {
  QSettings settings;
  this->remove_all_button();
  this->add_label_button("Next", settings.value("Next").toString());
  this->add_label_button("Prev", settings.value("Prev").toString());
  this->add_label_button("Delete", settings.value("Delete").toString());
  QStringList labels = settings.childGroups();
  for (QString label : labels) {
    settings.beginGroup(label);
    this->add_label_button(label, settings.value("shortcut").toString());
    settings.endGroup();
  }
}

void MainWindow::move_img_to(QString src, QString label) {
  QSettings settings;
  settings.beginGroup(label);
  QDir label_dir = QDir(this->dest.filePath(settings.value("dir").toString()));
  settings.endGroup();
  QString dest = QDir::cleanPath(label_dir.filePath(src));
  QFile src_file(this->dir.absoluteFilePath(src));
  src_file.rename(dest);
  qDebug() << "Move " << this->dir.absoluteFilePath(src) << " to " << dest;
  this->set_labels(this->img_idx, label);
  this->img_idx += 1;
  this->load_img_to_label();
}

void MainWindow::rm_img(QString src) {
  qDebug() << "Remove image" << src;
  QSettings settings;
  this->dir.remove(src);
  this->imgs.removeAt(this->img_idx);
  this->load_img_to_label();
}

void MainWindow::prev_img() {
  qDebug("Previouse Image");
  if (this->img_idx == 0) {
    qDebug("At head, cannot goto previous.");
    return;
  }
  QSettings settings;
  QString label = this->labels.at(this->img_idx - 1);
  if (label.isEmpty()) {
    qDebug("Directly goto previous image");
  } else {
    QString img_name = this->imgs.at(this->img_idx - 1);
    settings.beginGroup(label);
    QDir label_dir =
        QDir(this->dest.filePath(settings.value("dir").toString()));
    settings.endGroup();
    QString src = QDir::cleanPath(label_dir.filePath(img_name));
    QFile src_file(src);
    src_file.rename(this->dir.absoluteFilePath(img_name));
    qDebug() << "Move" << src << "back to"
             << this->dir.absoluteFilePath(img_name);
  }
  this->img_idx -= 1;
  this->load_img_to_label();
}

void MainWindow::next_img() {
  this->set_labels(this->img_idx, "");
  this->img_idx += 1;
  this->load_img_to_label();
}

void MainWindow::set_labels(int idx, QString label) {
  if (this->labels.size() > idx) {
    this->labels[idx] = label;
  } else {
    this->labels.append(label);
  }
}

void MainWindow::edit_setting() {
  QSettings settings;
  QDesktopServices::openUrl(
      QUrl(QString("file://%1").arg(settings.fileName()), QUrl::TolerantMode));
}

void MainWindow::restore_default_setting() {
  QSettings settings;
  QFile old_settings_file(settings.fileName());
  if (old_settings_file.remove()) {
    qDebug("Successfully delete settings file.");
    this->load_config();
  } else {
    qDebug("Failed to delete settings file.");
  }
}

void MainWindow::debug_msg_output(QtMsgType type,
                                  const QMessageLogContext &context,
                                  const QString &msg) {
  QByteArray localMsg = msg.toLocal8Bit();
  switch (type) {
  case QtDebugMsg:
    s_textBrowser->append(QString("Debug: %1\n").arg(localMsg.constData()));
    break;
  case QtInfoMsg:
    s_textBrowser->append(QString("Info: %1\n").arg(localMsg.constData()));
    break;
  case QtWarningMsg:
    s_textBrowser->append(QString("Warning: %1\n").arg(localMsg.constData()));
    break;
  case QtCriticalMsg:
    s_textBrowser->append(QString("Critical: %1\n").arg(localMsg.constData()));
    break;
  case QtFatalMsg:
    s_textBrowser->append(QString("Fatal: %1\n").arg(localMsg.constData()));
    abort();
  }
}

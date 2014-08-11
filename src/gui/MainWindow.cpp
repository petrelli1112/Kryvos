/**
 * Kryvos File Encryptor - Encrypts and decrypts files.
 * Copyright (C) 2014 Andrew Dolby
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact : andrewdolby@gmail.com
 */

#include "gui/MainWindow.hpp"
#include "gui/PasswordFrame.hpp"
#include "gui/HeaderFrame.hpp"
#include "gui/Delegate.hpp"
#include "utility/make_unique.h"
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMenu>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QTableView>
#include <QtWidgets/QHeaderView>
#include <QtGui/QIcon>
#include <QtGui/QStandardItemModel>
#include <QtGui/QStandardItem>
#include <QtCore/QModelIndexList>
#include <QtCore/QModelIndex>
#include <QtCore/QMimeData>
#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QUrl>
#include <QtCore/QSettings>
#include <QtCore/QSize>
#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QtCore/QString>

/*!
 * \brief MainWindowPrivate class
 */
class MainWindow::MainWindowPrivate {
 public:
  /*!
   * \brief MainWindowPrivate Constructs the MainWindow private implementation.
   * Initializes widgets, layouts, and settings.
   */
  explicit MainWindowPrivate();

  /*!
   * \brief loadStyleSheet Attempts to load a Qt stylesheet from the local
   * themes folder with the name specified in the local kryvos.ini file. If the
   * load fails, the method will load the default stylesheet from the
   * application resources.
   * \param styleFile String representing the name of the stylesheet without
   * a file extension.
   * \return String containing the stylesheet file contents.
   */
  QString loadStyleSheet(const QString& styleFile);

  /*!
   * \brief addFilePathToModel Adds a file to the model that represents the list
   * to be encrypted/decrypted.
   * \param filePath String representing the path to a file.
   */
  void addFilePathToModel(const QString& filePath);

  /*!
   * \brief clearModel Clears the model.
   */
  void clearModel();

  /*!
   * \brief busy Sets the busy status received from the cipher operation.
   * \param busy Boolean representing the busy status.
   */
  void busy(bool busy);

  /*!
   * \brief isBusy Returns the busy status received from the cipher operation.
   * \return Boolean representing the busy status.
   */
  bool isBusy() const;

  HeaderFrame* headerFrame;
  std::unique_ptr<QStandardItemModel> fileListModel;
  QTableView* fileListView;
  QPlainTextEdit* messageTextEdit;
  PasswordFrame* passwordFrame;

  // Messages to display to user
  const QStringList messages;

  // Settings strings
  QString lastDirectory;
  QString lastAlgorithmName;
  QString styleSheetPath;

 private:
  // The busy status, when set to true, indicates that this object is currently
  // executing a cipher operation. The status allows the GUI to decide whether
  // to send new encryption/decryption requests.
  bool busyStatus;
};

MainWindow::MainWindow(QWidget* parent) :
  QMainWindow{parent}, pimpl{make_unique<MainWindowPrivate>()}
{
  auto mainFrame = new QFrame{this};
  mainFrame->setObjectName("mainFrame");

  auto mainLayout = new QVBoxLayout{mainFrame};

  auto contentFrame = new QFrame{mainFrame};
  contentFrame->setObjectName("contentFrame");
  auto contentLayout = new QVBoxLayout{contentFrame};

  pimpl->headerFrame = new HeaderFrame{contentFrame};
  pimpl->headerFrame->setObjectName("headerFrame");
  contentLayout->addWidget(pimpl->headerFrame);

  // File list
  pimpl->fileListModel->setHeaderData(0, Qt::Horizontal, tr("Files"));
  pimpl->fileListModel->setHeaderData(1, Qt::Horizontal, tr("Progress"));
  pimpl->fileListModel->setHeaderData(2, Qt::Horizontal, tr("Remove file"));

  pimpl->fileListView = new QTableView{contentFrame};
  pimpl->fileListView->setModel(pimpl->fileListModel.get());

  QHeaderView* header = pimpl->fileListView->horizontalHeader();
  header->setStretchLastSection(false);

  header->hide();
  pimpl->fileListView->verticalHeader()->hide();
  pimpl->fileListView->setShowGrid(false);

  // Custom delegate paints progress bar and file close button for each file
  auto delegate = new Delegate{this};
  pimpl->fileListView->setItemDelegate(delegate);

  contentLayout->addWidget(pimpl->fileListView, 20);

  // Message text edit display
  auto messageFrame = new QFrame{contentFrame};
  messageFrame->setObjectName("message");
  messageFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  messageFrame->setContentsMargins(0, 0, 0, 0);

  pimpl->messageTextEdit->setParent(messageFrame);
  pimpl->messageTextEdit->setObjectName("message");
  pimpl->messageTextEdit->setReadOnly(true);
  pimpl->messageTextEdit->setTextInteractionFlags(Qt::NoTextInteraction);
  pimpl->messageTextEdit->viewport()->setCursor(Qt::ArrowCursor);
  pimpl->messageTextEdit->setSizePolicy(QSizePolicy::Expanding,
                                        QSizePolicy::Preferred);

  auto messageLayout = new QHBoxLayout{messageFrame};
  messageLayout->addWidget(pimpl->messageTextEdit);
  messageLayout->setContentsMargins(0, 0, 0, 0);
  messageLayout->setSpacing(0);

  contentLayout->addWidget(messageFrame, 1);

  // Password entry frame
  pimpl->passwordFrame = new PasswordFrame{contentFrame};
  contentLayout->addWidget(pimpl->passwordFrame);

  // Encrypt and decrypt control button frame
  auto buttonFrame = new QFrame{contentFrame};

  const auto lockIcon = QIcon{":/images/lockIcon.png"};
  auto encryptButton = new QPushButton{lockIcon,
                                       tr(" Encrypt"),
                                       buttonFrame};
  encryptButton->setIconSize(QSize{19, 19});
  encryptButton->setObjectName("cryptButton");
  encryptButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  const auto unlockedIcon = QIcon{":/images/unlockIcon.png"};
  auto decryptButton = new QPushButton{unlockedIcon,
                                       tr(" Decrypt"),
                                       buttonFrame};
  decryptButton->setIconSize(QSize{19, 19});
  decryptButton->setObjectName("cryptButton");
  decryptButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  auto buttonLayout = new QHBoxLayout{buttonFrame};
  buttonLayout->addWidget(encryptButton);
  buttonLayout->addWidget(decryptButton);

  contentLayout->addWidget(buttonFrame);

  mainLayout->addWidget(contentFrame);

  this->setCentralWidget(mainFrame);

  // Actions

  // Add files action
  auto addFilesAction = new QAction(this);
  addFilesAction->setShortcut(Qt::Key_O | Qt::CTRL);

  connect(addFilesAction, &QAction::triggered,
          this, &MainWindow::addFiles);
  this->addAction(addFilesAction);

  // Quit action
  auto quitAction = new QAction(this);
  quitAction->setShortcut(Qt::Key_Q | Qt::CTRL);

  connect(quitAction, &QAction::triggered,
          this, &QMainWindow::close);
  this->addAction(quitAction);

  // Header tool connections
  connect(pimpl->headerFrame, &HeaderFrame::pause,
          this, &MainWindow::pauseCipher);
  connect(pimpl->headerFrame, &HeaderFrame::addFiles,
          this, &MainWindow::addFiles);
  connect(pimpl->headerFrame, &HeaderFrame::removeFiles,
          this, &MainWindow::removeFiles);

  // Encryption connections
  connect(encryptButton, &QPushButton::clicked,
          this, &MainWindow::encryptFiles);
  connect(decryptButton, &QPushButton::clicked,
          this, &MainWindow::decryptFiles);

  connect(delegate, &Delegate::removeRow,
          this, &MainWindow::removeFileFromModel);

  // Set object name
  this->setObjectName("MainWindow");

  // Title
  this->setWindowTitle(tr("Kryvos"));

  // Read last window size and position from file
  this->importSettings();

  // Load stylesheet
  const auto styleSheet = pimpl->loadStyleSheet(pimpl->styleSheetPath);

  if (!styleSheet.isEmpty())
  {
    this->setStyleSheet(styleSheet);
  }

  // Enable drag and drop
  this->setAcceptDrops(true);
}

MainWindow::~MainWindow() {}

void MainWindow::addFiles()
{
  Q_ASSERT(pimpl);

  // Open a file dialog to get files
  const auto files = QFileDialog::getOpenFileNames(this,
                                                   tr("Add Files"),
                                                   pimpl->lastDirectory,
                                                   tr("Any files (*)"));

  if (!files.isEmpty())
  { // If files were selected, add them to the model
    for (const auto& file : files)
    {
      pimpl->addFilePathToModel(file);
    }

    // Save this directory to return to later
    const auto fileName = files[0];
    QFileInfo file{fileName};
    pimpl->lastDirectory = file.absolutePath();
  }
}

void MainWindow::removeFiles()
{
  Q_ASSERT(pimpl);

  // Signal to abort current cipher operation if it's in progress
  emit abortCipher();

  pimpl->clearModel();
}

void MainWindow::removeFileFromModel(const QModelIndex& index)
{
  Q_ASSERT(pimpl);
  Q_ASSERT(pimpl->fileListModel);

  auto testItem = pimpl->fileListModel->item(index.row(), 0);

  // Signal that this file shouldn't be encrypted or decrypted
  emit stopFile(testItem->text());

  // Remove row from model
  pimpl->fileListModel->removeRow(index.row());
}

void MainWindow::encryptFiles()
{
  Q_ASSERT(pimpl);
  Q_ASSERT(pimpl->passwordFrame->passwordLineEdit());
  Q_ASSERT(pimpl->fileListModel);

  if (!pimpl->isBusy())
  {
    // Get passphrase from line edit
    const QString passphrase{pimpl->passwordFrame->passwordLineEdit()->text()};

    if (!passphrase.isEmpty())
    {
      const auto rowCount = pimpl->fileListModel->rowCount();
      if (0 < rowCount)
      {
        QStringList fileList;

        for (int row = 0; row < rowCount; ++row)
        {
          auto item = pimpl->fileListModel->item(row, 0);
          fileList.append(item->text());
        }

        // Start encrypting the file list
        emit encrypt(passphrase, fileList, pimpl->lastAlgorithmName);
      }
    }
    else
    { // Inform user that a password is required to encrypt or decrypt
      this->updateStatusMessage(pimpl->messages[0]);
    }
  }
  else
  {
    this->updateStatusMessage(pimpl->messages[1]);
  }
}

void MainWindow::decryptFiles()
{
  Q_ASSERT(pimpl);
  Q_ASSERT(pimpl->passwordFrame->passwordLineEdit());
  Q_ASSERT(pimpl->fileListModel);

  if (!pimpl->isBusy())
  {
    // Get passphrase from line edit
    const QString passphrase{pimpl->passwordFrame->passwordLineEdit()->text()};

    if (!passphrase.isEmpty())
    {
      const int rowCount = pimpl->fileListModel->rowCount();
      if (0 < rowCount)
      {
        QStringList fileList;

        for (int row = 0; row < rowCount; ++row)
        {
          auto item = pimpl->fileListModel->item(row, 0);
          fileList.append(item->text());
        }

        // Start decrypting the file list
        emit decrypt(passphrase, fileList);
      }
    }
    else
    { // Inform user that a password is required to encrypt or decrypt
      this->updateStatusMessage(pimpl->messages[0]);
    }
  }
  else
  {
    this->updateStatusMessage(pimpl->messages[1]);
  }
}

void MainWindow::updateProgress(const QString& path, qint64 percent)
{
  Q_ASSERT(pimpl);
  Q_ASSERT(pimpl->fileListModel);

  QList<QStandardItem*> items = pimpl->fileListModel->findItems(path);

  if (0 < items.size())
  {
    auto item = items[0];

    if (nullptr != item)
    {
      const int index = item->row();

      auto progressItem = pimpl->fileListModel->item(index, 1);

      if (nullptr != progressItem)
      {
        progressItem->setData(percent, Qt::DisplayRole);
      }
    }
  }
}

void MainWindow::updateStatusMessage(const QString& message)
{
  Q_ASSERT(pimpl);
  Q_ASSERT(pimpl->messageTextEdit);

  pimpl->messageTextEdit->appendPlainText(message);
}

void MainWindow::updateError(const QString& path, const QString& message)
{
  this->updateStatusMessage(message);
  this->updateProgress(path, 0);
}

void MainWindow::updateBusyStatus(bool busy)
{
  Q_ASSERT(pimpl);

  pimpl->busy(busy);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
  this->exportSettings();

  QMainWindow::closeEvent(event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
  // Show drag and drop as a move action
  event->setDropAction(Qt::MoveAction);

  if (event->mimeData()->hasUrls())
  { // Accept drag and drops with files only
    event->accept();
  }
}

void MainWindow::dropEvent(QDropEvent* event)
{
  Q_ASSERT(pimpl);

  // Check for the URL MIME type, which is a list of files
  if (event->mimeData()->hasUrls())
  { // Extract the local path from the file(s)
    for (const auto& url : event->mimeData()->urls())
    {
      pimpl->addFilePathToModel(url.toLocalFile());
    }
  }
}

QSize MainWindow::sizeHint() const
{
  return QSize(800, 600);
}

QSize MainWindow::minimumSizeHint() const
{
  return QSize(600, 350);
}

void MainWindow::importSettings()
{
  Q_ASSERT(pimpl);

  QSettings settings{"settings.ini", QSettings::IniFormat};

  settings.beginGroup("MainWindow");

  if (settings.value("maximized").toBool())
  { // Move first to ensure maximize occurs on correct screen
    this->move(settings.value("maximizedPos", QPoint(200, 200)).toPoint());

    this->setWindowState(this->windowState() | Qt::WindowMaximized);
  }
  else
  {
    this->move(settings.value("pos", QPoint(200, 200)).toPoint());
    this->resize(settings.value("size", QSize(800, 600)).toSize());
  }

  pimpl->lastDirectory = settings.value("lastDirectory", "").toString();
  pimpl->lastAlgorithmName = settings.value("lastAlgorithmName",
                                            "AES-128/GCM").toString();
  pimpl->styleSheetPath = settings.value("styleSheetPath",
                                         "default/kryvos.qss").toString();

  settings.endGroup();
}

void MainWindow::exportSettings() const
{
  Q_ASSERT(pimpl);

  QSettings settings{"settings.ini", QSettings::IniFormat};

  settings.beginGroup("MainWindow");

  settings.setValue("maximized", this->isMaximized());

  if (!this->isMaximized())
  {
    settings.setValue("pos", this->pos());
    settings.setValue("size", this->size());
  }
  else
  { // Save position so maximize will occur on correct screen
    settings.setValue("maximizedPos", this->pos());
  }

  settings.setValue("lastDirectory", pimpl->lastDirectory);
  settings.setValue("lastAlgorithmName", pimpl->lastAlgorithmName);
  settings.setValue("styleSheetPath", pimpl->styleSheetPath);

  settings.endGroup();

  settings.sync();
}

MainWindow::MainWindowPrivate::MainWindowPrivate() :
  headerFrame{nullptr}, fileListModel{make_unique<QStandardItemModel>()},
  fileListView{nullptr},
  messageTextEdit{new QPlainTextEdit{tr("To add files, click the add files"
                                        " button or drag and drop files.")}},
  passwordFrame{nullptr},
  messages{tr("A password is required to encrypt or decrypt files. Please enter"
              " one to continue."),
           tr("Encryption/decryption is already in progress. Please wait until"
              " the current operation finishes.")},
  busyStatus{false} {}

QString MainWindow::MainWindowPrivate::loadStyleSheet(const QString& styleFile)
{
  // Try to load user theme, if it exists
  const auto styleSheetPath = QString{"themes/" + styleFile};
  QFile userTheme{styleSheetPath};

  QString styleSheet;

  if (userTheme.exists())
  {
    userTheme.open(QFile::ReadOnly);
    styleSheet = QLatin1String(userTheme.readAll());
    userTheme.close();
  }
  else
  { // Otherwise, load default theme
    QFile defaultTheme{":/stylesheets/kryvos.qss"};

    defaultTheme.open(QFile::ReadOnly);
    styleSheet = QLatin1String(defaultTheme.readAll());
    defaultTheme.close();
  }

  return styleSheet;
}

void MainWindow::MainWindowPrivate::addFilePathToModel(const QString& filePath)
{
  QFileInfo file{filePath};

  if (file.exists() && file.isFile())
  { // If the file exists, add it to the model
    auto pathItem = new QStandardItem{filePath};
    pathItem->setDragEnabled(false);
    pathItem->setDropEnabled(false);
    pathItem->setEditable(false);
    pathItem->setSelectable(false);

    auto progressItem = new QStandardItem{};
    progressItem->setDragEnabled(false);
    progressItem->setDropEnabled(false);
    progressItem->setEditable(false);
    progressItem->setSelectable(false);

    auto closeFileItem = new QStandardItem{};
    closeFileItem->setDragEnabled(false);
    closeFileItem->setDropEnabled(false);
    closeFileItem->setEditable(false);
    closeFileItem->setSelectable(false);

    QList<QStandardItem*> items;
    items.append(pathItem);
    items.append(progressItem);
    items.append(closeFileItem);

    if (0 == fileListModel->rowCount())
    { // Add right away if there are no items in the model
      fileListModel->appendRow(items);
      QHeaderView* header = fileListView->horizontalHeader();
      header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    }
    else
    { // Search to see if this item is already in the model
      bool addNewItem = true;

      const auto rowCount = fileListModel->rowCount();
      for (int row = 0; row < rowCount; ++row)
      {
        auto testItem = fileListModel->item(row, 0);

        if (testItem->text() == pathItem->text())
        {
          addNewItem = false;
        }
      }

      if (addNewItem)
      { // Add the item to the model if it's a new item
        fileListModel->appendRow(items);
        QHeaderView* header = fileListView->horizontalHeader();
        header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
      }
    } // End else
  } // End if file exists and is a file
}

void MainWindow::MainWindowPrivate::clearModel()
{
  fileListModel->clear();
}

void MainWindow::MainWindowPrivate::busy(bool busy)
{
  busyStatus = busy;
}

bool MainWindow::MainWindowPrivate::isBusy() const
{
  return busyStatus;
}

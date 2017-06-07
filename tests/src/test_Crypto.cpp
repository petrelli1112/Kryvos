#include "test_Crypto.hpp"
#include "FileOperations.h"
#include "src/cryptography/Crypto.hpp"
#include <QTest>
#include <QFile>
#include <QThread>
#include <QString>
#include <memory>

void TestCrypto::testComparatorSameFile() {
  // Test data
  const QString fileName1 = QStringLiteral("file1.png");
  const QString fileName2 = QStringLiteral("file2.png");

  QFile file1{fileName1};
  QFile file2{fileName2};
  if (!file1.exists() || !file2.exists()) {
    QString message;
    const QString msg = QStringLiteral("Test file %1 is missing. ");

    if (!file1.exists()) {
      message += msg.arg(fileName1);
    }

    if (!file2.exists()) {
      message += msg.arg(fileName2);
    }

    QSKIP(message.toStdString().c_str());
  }

  const bool equivalentTest = FileOperations::filesEqual(fileName1, fileName2);

  QVERIFY(equivalentTest);
}

void TestCrypto::testComparatorDifferentFile() {
  // Test data
  const QString fileName1 = QStringLiteral("file1.png");
  const QString fileName2 = QStringLiteral("file3.png");

  QFile file1{fileName1};
  QFile file2{fileName2};
  if (!file1.exists() || !file2.exists()) {
    QString message;
    const QString msg = QStringLiteral("Test file %1 is missing. ");

    if (!file1.exists()) {
      message += msg.arg(fileName1);
    }

    if (!file2.exists()) {
      message += msg.arg(fileName2);
    }

    QSKIP(message.toStdString().c_str());
  }

  const bool equivalentTest = FileOperations::filesEqual(fileName1, fileName2);

  QVERIFY(!equivalentTest);
}

void TestCrypto::testEncryptDecrypt_data() {
  QTest::addColumn<QString>("passphrase");
  QTest::addColumn<QString>("cipher");
  QTest::addColumn<int>("keySize");
  QTest::addColumn<QString>("modeOfOperation");
  QTest::addColumn<bool>("compress");
  QTest::addColumn<QString>("inputFileName");
  QTest::addColumn<QString>("encryptedFileName");
  QTest::addColumn<QString>("decryptedFileName");

  QTest::newRow("Text file with compression")
    << "password" << "AES" << 128 << "GCM" << true << "test.txt"
    << "test.txt.enc" << "test (2).txt";

  QTest::newRow("Executable file with compression")
    << "password2" << "AES" << 128 << "GCM" << true << "test.exe"
    << "test.exe.enc" << "test (2).exe";

  QTest::newRow("Zip file with compression")
    << "password3" << "AES" << 128 << "GCM" << true << "test.zip"
    << "test.zip.enc" << "test (2).zip";

  QTest::newRow("Text file without compression")
    << "password" << "AES" << 128 << "GCM" << true << "test.txt"
    << "test.txt.enc" << "test (2).txt";

  QTest::newRow("Executable file without compression")
    << "password2" << "AES" << 128 << "GCM" << true << "test.exe"
    << "test.exe.enc" << "test (2).exe";

  QTest::newRow("Zip file without compression")
    << "password3" << "AES" << 128 << "GCM" << true << "test.zip"
    << "test.zip.enc" << "test (2).zip";
}

void TestCrypto::testEncryptDecrypt() {
  QFETCH(QString, passphrase);
  QFETCH(QString, cipher);
  QFETCH(int, keySize);
  QFETCH(QString, modeOfOperation);
  QFETCH(bool, compress);
  QFETCH(QString, inputFileName);
  QFETCH(QString, encryptedFileName);
  QFETCH(QString, decryptedFileName);

  QFile inputFile{inputFileName};
  if (!inputFile.exists()) {
    const QString msg = QStringLiteral("Test file %1 is missing.");
    QSKIP(msg.arg(inputFileName).toStdString().c_str());
  }

  const QStringList inputFileNames = {inputFileName};
  const QStringList encryptedFileNames = {encryptedFileName};

  Kryvos::Crypto cryptography{};

  const QString outputPath = "";

  // Test encryption and decryption
  cryptography.encrypt(passphrase, inputFileNames, outputPath, cipher,
                       static_cast<std::size_t>(keySize), modeOfOperation,
                       compress, false);
  cryptography.decrypt(passphrase, encryptedFileNames);

  // Compare initial file to decrypted file
  const bool equivalentTest =
        FileOperations::filesEqual(inputFileName, decryptedFileName);

  // Clean up test files
  QFile encryptedFile{encryptedFileName};
  if (encryptedFile.exists()) {
    encryptedFile.remove();
  }

  QFile decryptedFile{decryptedFileName};
  if (decryptedFile.exists()) {
    decryptedFile.remove();
  }

  QVERIFY(equivalentTest);
}

void TestCrypto::testEncryptDecryptAll() {
  // Test data
  const QString passphrase = QStringLiteral("password");
  const QString cipher = QStringLiteral("AES");
  const std::size_t keySize = std::size_t{128};
  const QString modeOfOperation = QStringLiteral("GCM");
  const bool compress = true;

  const QStringList inputFileNames = {"test.txt",
                                      "test.exe",
                                      "test.zip"};

  bool skip = false;
  QString message;

  foreach (const QString& inputFileName, inputFileNames) {
    QFile inputFile{inputFileName};

    if (!inputFile.exists()) {
      message += QString{"\nTest file %1 is missing."}.arg(inputFileName);
      skip = true;
    }
  }

  if (skip) {
    QSKIP(message.toStdString().c_str());
  }

  const QStringList encryptedFileNames = {"test.txt.enc",
                                          "test.exe.enc",
                                          "test.zip.enc"};

  const QStringList decryptedFileNames = {"test (2).txt",
                                          "test (2).exe",
                                          "test (2).zip"};

  Kryvos::Crypto cryptography{};

  const QString outputPath = "";

  // Test encryption and decryption
  cryptography.encrypt(passphrase, inputFileNames, outputPath, cipher, keySize,
                       modeOfOperation, compress, false);
  cryptography.decrypt(passphrase, encryptedFileNames);

  bool equivalentTest = false;

  const int inputFilesSize = inputFileNames.size();
  for (auto i = 0; i < inputFilesSize; ++i) {
    const QString inputFileName = inputFileNames[i];
    const QString decryptedFileName = decryptedFileNames[i];

    // Compare initial file to decrypted file
    equivalentTest =
          FileOperations::filesEqual(inputFileName, decryptedFileName);

    if (!equivalentTest) { // If these two files weren't equivalent, test failed
      break;
    }
  }

  // Clean up test files
  for (auto i = 0; i < inputFilesSize; ++i) {
    const QString encryptedFileName = encryptedFileNames[i];
    QFile encryptedFile{encryptedFileName};

    if (encryptedFile.exists()) {
      encryptedFile.remove();
    }

    const QString decryptedFileName = decryptedFileNames[i];
    QFile decryptedFile{decryptedFileName};

    if (decryptedFile.exists()) {
      decryptedFile.remove();
    }
  }

  QVERIFY(equivalentTest);
}

QTEST_GUILESS_MAIN(TestCrypto)

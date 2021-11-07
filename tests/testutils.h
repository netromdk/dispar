#ifndef DISPAR_TESTUTILS_H
#define DISPAR_TESTUTILS_H

#include <QByteArray>
#include <QFile>

#include <functional>
#include <memory>
#include <ostream>

/// Creates temporary file that will be deleted when pointer is destroyed.
/** If \p data is specified it will be written to the file. */
std::unique_ptr<QFile, std::function<void(QFile *)>>
tempFile(const QByteArray &data = QByteArray());

/// Generates a random, temporary file path and makes sure it doesn't exist.
QString tempFilePath();

std::ostream &operator<<(std::ostream &os, const QString &str);

#endif // DISPAR_TESTUTILS_H

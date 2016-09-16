#ifndef DISPAR_TESTUTILS_H
#define DISPAR_TESTUTILS_H

#include <QFile>
#include <QByteArray>

#include <memory>
#include <functional>

/// Creates temporary file that will be deleted when pointer is destroyed.
/** If \p data is specified it will be written to the file. */
std::unique_ptr<QFile, std::function<void(QFile *)>>
tempFile(const QByteArray &data = QByteArray());

#endif // DISPAR_TESTUTILS_H

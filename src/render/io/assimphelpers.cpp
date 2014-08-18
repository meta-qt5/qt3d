/****************************************************************************
**
** Copyright (C) 2014 Klaralvdalens Datakonsult AB (KDAB).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "assimphelpers_p.h"

#include <QFileInfo>
#include <QUrl>
#include <QDir>
#include <QDebug>
#include <QMap>

QT_BEGIN_NAMESPACE

namespace Qt3D {
namespace AssimpHelper {
/*!
 *  \class AssimpIOStream
 *
 *  \internal
 *
 *  \brief Provides a custom file stream class to be used by AssimpIOSystem.
 *
 */

/*!
 *  Builds a new AssimpIOStream instance.
 */
AssimpIOStream::AssimpIOStream(QIODevice *device) :
    Assimp::IOStream(),
    m_device(device)
{
}

/*!
 *  Clears an AssimpIOStream instance before deletion.
 */
AssimpIOStream::~AssimpIOStream()
{
    // Owns m_device
    if (m_device && m_device->isOpen()) {
        m_device->close();
        delete m_device;
    }
}

/*!
 *  Reads at most \a pCount elements of \a pSize bytes of data into \a pvBuffer.
 *  Returns the number of bytes read or -1 if an error occurred.
 */
size_t AssimpIOStream::Read(void *pvBuffer, size_t pSize, size_t pCount)
{
    qint64 readBytes = m_device->read((char *)pvBuffer, pSize * pCount);
    if (readBytes < 0)
        qWarning() << Q_FUNC_INFO << " Reading failed";
    return readBytes;
}


/*!
 * Writes \a pCount elements of \a pSize bytes from \a pvBuffer.
 * Returns the number of bytes written or -1 if an error occurred.
 */
size_t AssimpIOStream::Write(const void *pvBuffer, size_t pSize, size_t pCount)
{
    qint64 writtenBytes = m_device->write((char *)pvBuffer, pSize * pCount);
    if (writtenBytes < 0)
        qWarning() << Q_FUNC_INFO << " Writing failed";
    return writtenBytes;
}

/*!
 * Seeks the current file descriptor to a position defined by \a pOrigin and \a pOffset
 */
aiReturn AssimpIOStream::Seek(size_t pOffset, aiOrigin pOrigin)
{
    qint64 seekPos = pOffset;

    if (pOrigin == aiOrigin_CUR)
        seekPos += m_device->pos();
    else if (pOrigin == aiOrigin_END)
        seekPos += m_device->size();

    if (!m_device->seek(seekPos)) {
        qWarning() << Q_FUNC_INFO << " Seeking failed";
        return aiReturn_FAILURE;
    }
    return aiReturn_SUCCESS;
}

/*!
 * Returns the current position of the read/write cursor.
 */
size_t AssimpIOStream::Tell() const
{
    return m_device->pos();
}

/*!
 * Returns the filesize;
 */
size_t AssimpIOStream::FileSize() const
{
    return m_device->size();
}

/*!
 * Flushes the current device.
 */
void AssimpIOStream::Flush()
{
    // QIODevice has no flush method
}

/*!
 * \class AssimpIOSystem
 *
 * \internal
 *
 * \brief Provides a custom file handler to the Assimp importer in order to handle
 * various Qt specificities (QResources ...)
 *
 */

/*!
 * Builds a new instance of AssimpIOSystem.
 */
AssimpIOSystem::AssimpIOSystem() :
    Assimp::IOSystem()
{
    m_openModeMaps[QStringLiteral("r")] = QIODevice::ReadOnly;
    m_openModeMaps[QStringLiteral("r+")] = QIODevice::ReadWrite;
    m_openModeMaps[QStringLiteral("w")] = QIODevice::WriteOnly|QIODevice::Truncate;
    m_openModeMaps[QStringLiteral("w+")] = QIODevice::ReadWrite|QIODevice::Truncate;
    m_openModeMaps[QStringLiteral("a")] = QIODevice::WriteOnly|QIODevice::Append;
    m_openModeMaps[QStringLiteral("a+")] = QIODevice::ReadWrite|QIODevice::Append;
    m_openModeMaps[QStringLiteral("wb")] = QIODevice::WriteOnly;
    m_openModeMaps[QStringLiteral("wt")] = QIODevice::WriteOnly;
    m_openModeMaps[QStringLiteral("rb")] = QIODevice::ReadOnly;
    m_openModeMaps[QStringLiteral("rt")] = QIODevice::ReadOnly;
}

/*!
 * Clears an AssimpIOSystem instance before deletion.
 */
AssimpIOSystem::~AssimpIOSystem()
{
}

/*!
 * Returns true if the file located at pFile exists, false otherwise.
 */
bool AssimpIOSystem::Exists(const char *pFile) const
{
    return QFileInfo(QString::fromUtf8(pFile)).exists();
}

/*!
 * Returns the default system separator.
 */
char AssimpIOSystem::getOsSeparator() const
{
    return QDir::separator().toLatin1();
}

/*!
 * Opens the file located at \a pFile with the opening mode
 * specified by \a pMode.
 */
Assimp::IOStream *AssimpIOSystem::Open(const char *pFile, const char *pMode)
{
    QIODevice::OpenMode openMode = QIODevice::NotOpen;
    QString cleanedMode = QString::fromUtf8(pMode).trimmed();
    if (m_openModeMaps.contains(cleanedMode))
        openMode = m_openModeMaps[cleanedMode];

    QFile *file = new QFile(QString::fromUtf8(pFile));
    if (file->open(openMode))
        return new AssimpIOStream(file);
    delete file;
    return NULL;
}

/*!
 * Closes the Assimp::IOStream \a pFile.
 */
void AssimpIOSystem::Close(Assimp::IOStream *pFile)
{
    // Assimp::IOStream has a virtual destructor which closes the stream
    delete pFile;
}

} // AssimpHelper namespace
} // Qt3D namespace

QT_END_NAMESPACE

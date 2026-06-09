#include "ImageListModel.h"

#include <algorithm>
#include <QDir>
#include <QDirIterator>
#include <QImageReader>
#include <QSet>

ImageListModel::ImageListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int ImageListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_files.size();
}

QVariant ImageListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_files.size()) {
        return {};
    }

    const QFileInfo &file = m_files.at(index.row());
    switch (role) {
    case FilePathRole:
        return file.absoluteFilePath();
    case FileUrlRole:
        return QUrl::fromLocalFile(file.absoluteFilePath());
    case FileNameRole:
        return file.fileName();
    case StatusRole:
        return QStringLiteral("ready");
    default:
        return {};
    }
}

QHash<int, QByteArray> ImageListModel::roleNames() const
{
    return {
        {FilePathRole, "filePath"},
        {FileUrlRole, "fileUrl"},
        {FileNameRole, "fileName"},
        {StatusRole, "status"},
    };
}

int ImageListModel::count() const
{
    return m_files.size();
}

bool ImageListModel::loadFolder(const QUrl &folderUrl)
{
    const QString folderPath = folderUrl.isLocalFile() ? folderUrl.toLocalFile() : folderUrl.toString();
    const QDir folder(folderPath);
    if (!folder.exists()) {
        emit folderLoadFailed(tr("Folder does not exist: %1").arg(folderPath));
        return false;
    }

    QList<QFileInfo> discovered;
    QDirIterator iterator(folder.absolutePath(), QDir::Files, QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        const QFileInfo info(iterator.next());
        if (isSupportedImage(info)) {
            discovered.append(info);
        }
    }

    std::sort(discovered.begin(), discovered.end(), [](const QFileInfo &left, const QFileInfo &right) {
        return left.absoluteFilePath().localeAwareCompare(right.absoluteFilePath()) < 0;
    });

    beginResetModel();
    m_files = discovered;
    endResetModel();
    emit countChanged();
    return true;
}

void ImageListModel::clear()
{
    if (m_files.isEmpty()) {
        return;
    }

    beginResetModel();
    m_files.clear();
    endResetModel();
    emit countChanged();
}

QString ImageListModel::itemPath(int index) const
{
    if (index < 0 || index >= m_files.size()) {
        return {};
    }
    return m_files.at(index).absoluteFilePath();
}

QUrl ImageListModel::itemUrl(int index) const
{
    const QString path = itemPath(index);
    return path.isEmpty() ? QUrl() : QUrl::fromLocalFile(path);
}

QStringList ImageListModel::paths() const
{
    QStringList result;
    result.reserve(m_files.size());
    for (const QFileInfo &file : m_files) {
        result.append(file.absoluteFilePath());
    }
    return result;
}

bool ImageListModel::isSupportedImage(const QFileInfo &fileInfo)
{
    static const QSet<QByteArray> supportedFormats = [] {
        QSet<QByteArray> formats;
        const QList<QByteArray> readerFormats = QImageReader::supportedImageFormats();
        for (const QByteArray &format : readerFormats) {
            formats.insert(format.toLower());
        }
        return formats;
    }();

    return supportedFormats.contains(fileInfo.suffix().toLower().toUtf8());
}

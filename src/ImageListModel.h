#pragma once

#include <QAbstractListModel>
#include <QFileInfo>
#include <QList>
#include <QStringList>
#include <QUrl>

class ImageListModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    enum Role {
        FilePathRole = Qt::UserRole + 1,
        FileUrlRole,
        FileNameRole,
        StatusRole
    };
    Q_ENUM(Role)

    explicit ImageListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int count() const;

    Q_INVOKABLE bool loadFolder(const QUrl &folderUrl);
    Q_INVOKABLE void clear();
    Q_INVOKABLE QString itemPath(int index) const;
    Q_INVOKABLE QUrl itemUrl(int index) const;
    Q_INVOKABLE QStringList paths() const;

signals:
    void countChanged();
    void folderLoadFailed(const QString &message);

private:
    static bool isSupportedImage(const QFileInfo &fileInfo);

    QList<QFileInfo> m_files;
};

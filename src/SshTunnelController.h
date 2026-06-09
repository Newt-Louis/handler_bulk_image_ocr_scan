#pragma once

#include <QObject>

class SshTunnelController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString serverHost READ serverHost WRITE setServerHost NOTIFY serverHostChanged)
    Q_PROPERTY(QString serverUser READ serverUser WRITE setServerUser NOTIFY serverUserChanged)
    Q_PROPERTY(QString remoteHost READ remoteHost WRITE setRemoteHost NOTIFY remoteHostChanged)
    Q_PROPERTY(int remotePort READ remotePort WRITE setRemotePort NOTIFY remotePortChanged)
    Q_PROPERTY(int localPort READ localPort WRITE setLocalPort NOTIFY localPortChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString commandPreview READ commandPreview NOTIFY commandPreviewChanged)

public:
    explicit SshTunnelController(QObject *parent = nullptr);

    QString serverHost() const;
    QString serverUser() const;
    QString remoteHost() const;
    int remotePort() const;
    int localPort() const;
    bool running() const;
    QString statusText() const;
    QString commandPreview() const;

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

public slots:
    void setServerHost(const QString &serverHost);
    void setServerUser(const QString &serverUser);
    void setRemoteHost(const QString &remoteHost);
    void setRemotePort(int remotePort);
    void setLocalPort(int localPort);

signals:
    void serverHostChanged();
    void serverUserChanged();
    void remoteHostChanged();
    void remotePortChanged();
    void localPortChanged();
    void runningChanged();
    void statusTextChanged();
    void commandPreviewChanged();
    void failed(const QString &message);

private:
    void setRunning(bool running);
    void setStatusText(const QString &statusText);
    void emitCommandPreviewChanged();

    QString m_serverHost;
    QString m_serverUser;
    QString m_remoteHost = QStringLiteral("127.0.0.1");
    int m_remotePort = 8080;
    int m_localPort = 18080;
    bool m_running = false;
    QString m_statusText = QStringLiteral("Tunnel placeholder idle");
};

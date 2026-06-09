#include "SshTunnelController.h"

#include <QtGlobal>

SshTunnelController::SshTunnelController(QObject *parent)
    : QObject(parent)
{
}

QString SshTunnelController::serverHost() const
{
    return m_serverHost;
}

QString SshTunnelController::serverUser() const
{
    return m_serverUser;
}

QString SshTunnelController::remoteHost() const
{
    return m_remoteHost;
}

int SshTunnelController::remotePort() const
{
    return m_remotePort;
}

int SshTunnelController::localPort() const
{
    return m_localPort;
}

bool SshTunnelController::running() const
{
    return m_running;
}

QString SshTunnelController::statusText() const
{
    return m_statusText;
}

QString SshTunnelController::commandPreview() const
{
    const QString destination = m_serverUser.trimmed().isEmpty()
        ? m_serverHost.trimmed()
        : QStringLiteral("%1@%2").arg(m_serverUser.trimmed(), m_serverHost.trimmed());

    if (destination.isEmpty()) {
        return QStringLiteral("ssh -N -L <local>:<remote-host>:<remote-port> <user>@<server>");
    }

    return QStringLiteral("ssh -N -L %1:%2:%3 %4")
        .arg(m_localPort)
        .arg(m_remoteHost.trimmed().isEmpty() ? QStringLiteral("127.0.0.1") : m_remoteHost.trimmed())
        .arg(m_remotePort)
        .arg(destination);
}

void SshTunnelController::start()
{
    if (m_serverHost.trimmed().isEmpty()) {
        const QString message = tr("SSH server host is empty.");
        setStatusText(message);
        emit failed(message);
        return;
    }

    setRunning(true);
    setStatusText(tr("SSH tunnel placeholder active; no SSH process is started yet."));
}

void SshTunnelController::stop()
{
    if (!m_running) {
        return;
    }

    setRunning(false);
    setStatusText(tr("SSH tunnel placeholder stopped."));
}

void SshTunnelController::setServerHost(const QString &serverHost)
{
    if (m_serverHost == serverHost) {
        return;
    }
    m_serverHost = serverHost;
    emit serverHostChanged();
    emitCommandPreviewChanged();
}

void SshTunnelController::setServerUser(const QString &serverUser)
{
    if (m_serverUser == serverUser) {
        return;
    }
    m_serverUser = serverUser;
    emit serverUserChanged();
    emitCommandPreviewChanged();
}

void SshTunnelController::setRemoteHost(const QString &remoteHost)
{
    if (m_remoteHost == remoteHost) {
        return;
    }
    m_remoteHost = remoteHost;
    emit remoteHostChanged();
    emitCommandPreviewChanged();
}

void SshTunnelController::setRemotePort(int remotePort)
{
    const int clamped = qBound(1, remotePort, 65535);
    if (m_remotePort == clamped) {
        return;
    }
    m_remotePort = clamped;
    emit remotePortChanged();
    emitCommandPreviewChanged();
}

void SshTunnelController::setLocalPort(int localPort)
{
    const int clamped = qBound(1, localPort, 65535);
    if (m_localPort == clamped) {
        return;
    }
    m_localPort = clamped;
    emit localPortChanged();
    emitCommandPreviewChanged();
}

void SshTunnelController::setRunning(bool running)
{
    if (m_running == running) {
        return;
    }
    m_running = running;
    emit runningChanged();
}

void SshTunnelController::setStatusText(const QString &statusText)
{
    if (m_statusText == statusText) {
        return;
    }
    m_statusText = statusText;
    emit statusTextChanged();
}

void SshTunnelController::emitCommandPreviewChanged()
{
    emit commandPreviewChanged();
}

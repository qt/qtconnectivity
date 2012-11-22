#include "qbluetoothsocket.h"
#include "qbluetoothsocket_p.h"
#include "qbluetoothlocaldevice.h"
#include <sys/stat.h>

QTBLUETOOTH_BEGIN_NAMESPACE

QBluetoothSocketPrivate::QBluetoothSocketPrivate()
    : socket(-1),
      socketType(QBluetoothSocket::UnknownSocketType),
      state(QBluetoothSocket::UnconnectedState),
      readNotifier(0),
      connectWriteNotifier(0),
      connecting(false),
      discoveryAgent(0),
      isServerSocket(false)
{
    ppsRegisterControl();
}

QBluetoothSocketPrivate::~QBluetoothSocketPrivate()
{
    ppsUnregisterControl();
    close();
}

bool QBluetoothSocketPrivate::ensureNativeSocket(QBluetoothSocket::SocketType type)
{
    Q_UNUSED(type);
    return false;
}

void QBluetoothSocketPrivate::connectToService(const QBluetoothAddress &address, QBluetoothUuid uuid, QIODevice::OpenMode openMode)
{
    Q_UNUSED(openMode);
    if (isServerSocket) {
        m_peerAddress = address;
        m_uuid = uuid;
        return;
    }
    qBBBluetoothDebug() << "Connecting..";
    if (state != QBluetoothSocket::UnconnectedState) {
        qBBBluetoothDebug() << "Socket already connected";
        return;
    }
    state = QBluetoothSocket::ConnectingState;

    m_uuid = uuid;
    m_peerAddress = address;

    ppsSendControlMessage("connect_service", 0x1101, uuid, address.toString(), this);
    ppsRegisterForEvent(QStringLiteral("service_connected"),this);
    ppsRegisterForEvent(QStringLiteral("get_mount_point_path"),this);
    socketType = QBluetoothSocket::RfcommSocket;
}

void QBluetoothSocketPrivate::_q_writeNotify()
{
    Q_Q(QBluetoothSocket);
    if (connecting && state == QBluetoothSocket::ConnectingState){
        q->setSocketState(QBluetoothSocket::ConnectedState);
        emit q->connected();

        connectWriteNotifier->setEnabled(false);
        connecting = false;
    } else {
        if (txBuffer.size() == 0) {
            connectWriteNotifier->setEnabled(false);
            return;
        }

        char buf[1024];
        Q_Q(QBluetoothSocket);

        int size = txBuffer.read(buf, 1024);

        if (::write(socket, buf, size) != size) {
            socketError = QBluetoothSocket::NetworkError;
            emit q->error(socketError);
        }
        else {
            emit q->bytesWritten(size);
        }

        if (txBuffer.size()) {
            connectWriteNotifier->setEnabled(true);
        }
        else if (state == QBluetoothSocket::ClosingState) {
            connectWriteNotifier->setEnabled(false);
            this->close();
        }
    }
}

void QBluetoothSocketPrivate::_q_readNotify()
{
    Q_Q(QBluetoothSocket);
    char *writePointer = buffer.reserve(QPRIVATELINEARBUFFER_BUFFERSIZE);
    int readFromDevice = ::read(socket, writePointer, QPRIVATELINEARBUFFER_BUFFERSIZE);
    if (readFromDevice <= 0){
        int errsv = errno;
        readNotifier->setEnabled(false);
        connectWriteNotifier->setEnabled(false);
        errorString = QString::fromLocal8Bit(strerror(errsv));
        qDebug() << Q_FUNC_INFO << socket << " error:" << readFromDevice << errorString; //TODO Try if this actually works
        emit q->error(QBluetoothSocket::UnknownSocketError);

        q->disconnectFromService();
        q->setSocketState(QBluetoothSocket::UnconnectedState);
    } else {
        buffer.chop(QPRIVATELINEARBUFFER_BUFFERSIZE - (readFromDevice < 0 ? 0 : readFromDevice));

        emit q->readyRead();
    }
}

void QBluetoothSocketPrivate::abort()
{
    ppsSendControlMessage("disconnect_service", 0x1101, m_uuid, m_peerAddress.toString(), this);
    delete readNotifier;
    readNotifier = 0;
    delete connectWriteNotifier;
    connectWriteNotifier = 0;

    ::close(socket);

    Q_Q(QBluetoothSocket);
    emit q->disconnected();
    isServerSocket = false;
}

QString QBluetoothSocketPrivate::localName() const
{
    QBluetoothLocalDevice ld;
    return ld.name();
}

QBluetoothAddress QBluetoothSocketPrivate::localAddress() const
{
    QBluetoothLocalDevice ld;
    return ld.address();
}

quint16 QBluetoothSocketPrivate::localPort() const
{
    return 0;
}

QString QBluetoothSocketPrivate::peerName() const
{
    return m_peerName;
}

QBluetoothAddress QBluetoothSocketPrivate::peerAddress() const
{
    return m_peerAddress;
}

quint16 QBluetoothSocketPrivate::peerPort() const
{
    return 0;
}

qint64 QBluetoothSocketPrivate::writeData(const char *data, qint64 maxSize)
{
    Q_Q(QBluetoothSocket);
    if (q->openMode() & QIODevice::Unbuffered) {
        if (::write(socket, data, maxSize) != maxSize) {
            socketError = QBluetoothSocket::NetworkError;
            qWarning() << Q_FUNC_INFO << "Socket error";
            Q_EMIT q->error(socketError);
        }

        Q_EMIT q->bytesWritten(maxSize);

        return maxSize;
    } else {
        if (!connectWriteNotifier)
            return 0;

        if (txBuffer.size() == 0) {
            connectWriteNotifier->setEnabled(true);
            QMetaObject::invokeMethod(q, "_q_writeNotify", Qt::QueuedConnection);
        }

        char *txbuf = txBuffer.reserve(maxSize);
        memcpy(txbuf, data, maxSize);

        return maxSize;
    }
}

qint64 QBluetoothSocketPrivate::readData(char *data, qint64 maxSize)
{
    if (!buffer.isEmpty()){
        int i = buffer.read(data, maxSize);
        return i;
    }
    return 0;
}

void QBluetoothSocketPrivate::close()
{
    Q_Q(QBluetoothSocket);

    ppsSendControlMessage("disconnect_service", 0x1101, m_uuid, peerAddress().toString(), this);

    // Only go through closing if the socket was fully opened
    if (state == QBluetoothSocket::ConnectedState)
        q->setSocketState(QBluetoothSocket::ClosingState);

    if (txBuffer.size() > 0 && state == QBluetoothSocket::ClosingState) {
        connectWriteNotifier->setEnabled(true);
    } else {

        delete readNotifier;
        readNotifier = 0;
        delete connectWriteNotifier;
        connectWriteNotifier = 0;

        // We are disconnected now, so go to unconnected.
        q->setSocketState(QBluetoothSocket::UnconnectedState);
        Q_EMIT q->disconnected();
        ::close(socket);
    }
}

bool QBluetoothSocketPrivate::setSocketDescriptor(int socketDescriptor, QBluetoothSocket::SocketType socketType,
                                                  QBluetoothSocket::SocketState socketState, QBluetoothSocket::OpenMode openMode)
{
    Q_Q(QBluetoothSocket);
    delete readNotifier;
    readNotifier = 0;
    delete connectWriteNotifier;
    connectWriteNotifier = 0;

    socket = socketDescriptor;
    socketType = socketType;

    // ensure that O_NONBLOCK is set on new connections.
    int flags = fcntl(socket, F_GETFL, 0);
    if (!(flags & O_NONBLOCK))
        fcntl(socket, F_SETFL, flags | O_NONBLOCK);

    readNotifier = new QSocketNotifier(socket, QSocketNotifier::Read);
    QObject::connect(readNotifier, SIGNAL(activated(int)), q, SLOT(_q_readNotify()));
    connectWriteNotifier = new QSocketNotifier(socket, QSocketNotifier::Write, q);
    QObject::connect(connectWriteNotifier, SIGNAL(activated(int)), q, SLOT(_q_writeNotify()));

    q->setSocketState(socketState);
    q->setOpenMode(openMode);

    if (openMode == QBluetoothSocket::ConnectedState)
        emit q->connected();

    isServerSocket = true;

    return true;
}

int QBluetoothSocketPrivate::socketDescriptor() const
{
    return 0;
}

qint64 QBluetoothSocketPrivate::bytesAvailable() const
{
    return buffer.size();
}

void QBluetoothSocketPrivate::controlReply(ppsResult result)
{
    Q_Q(QBluetoothSocket);

    if (result.msg == QStringLiteral("connect_service")) {
        if (!result.errorMsg.isEmpty()) {
            qWarning() << Q_FUNC_INFO << "Error connecting to service:" << result.errorMsg;
            errorString = result.errorMsg;
            socketError = QBluetoothSocket::UnknownSocketError;
            emit q->error(QBluetoothSocket::UnknownSocketError);
            q->setSocketState(QBluetoothSocket::UnconnectedState);
            return;
        }

    } else if (result.msg == QStringLiteral("get_mount_point_path")) {
        QString path;
        path = result.dat.first();
        qBBBluetoothDebug() << Q_FUNC_INFO << "PATH is" << path;

        if (!result.errorMsg.isEmpty()) {
            qWarning() << Q_FUNC_INFO << result.errorMsg;
            errorString = result.errorMsg;
            socketError = QBluetoothSocket::UnknownSocketError;
            emit q->error(QBluetoothSocket::UnknownSocketError);
            q->setSocketState(QBluetoothSocket::UnconnectedState);
            return;
        } else {
            qBBBluetoothDebug() << "Mount point path is:" << path;
            socket = ::open(path.toStdString().c_str(), O_RDWR);
            if (socket == -1) {
                errorString = QString::fromLocal8Bit(strerror(errno));
                qDebug() << Q_FUNC_INFO << socket << " error:" << errno << errorString; //TODO Try if this actually works
                emit q->error(QBluetoothSocket::UnknownSocketError);

                q->disconnectFromService();
                q->setSocketState(QBluetoothSocket::UnconnectedState);
                return;
            }

            Q_Q(QBluetoothSocket);
            readNotifier = new QSocketNotifier(socket, QSocketNotifier::Read);
            QObject::connect(readNotifier, SIGNAL(activated(int)), q, SLOT(_q_readNotify()));
            connectWriteNotifier = new QSocketNotifier(socket, QSocketNotifier::Write, q);
            QObject::connect(connectWriteNotifier, SIGNAL(activated(int)), q, SLOT(_q_writeNotify()));

            connectWriteNotifier->setEnabled(true);
            readNotifier->setEnabled(true);
            emit q->connected();
        }
    }
}

void QBluetoothSocketPrivate::controlEvent(ppsResult result)
{
    if (result.msg == QStringLiteral("service_connected")) {
        qBBBluetoothDebug() << Q_FUNC_INFO << "Sending request for mount point";
        ppsSendControlMessage("get_mount_point_path", 0x1101, m_uuid, m_peerAddress.toString(), this);
        return;
    }
}

QTBLUETOOTH_END_NAMESPACE


#include <QCoreApplication>
#include <QLocalSocket>
#include <QSocketNotifier>
#include <QTimer>

class NamedPipeClient: public QObject {
    //Q_OBJECT
public:
    NamedPipeClient(QObject *parent = nullptr) {
        socket = new QLocalSocket();
        connect(socket, &QLocalSocket::readyRead, this, &NamedPipeClient::handleReadyRead);
        connect(socket, &QLocalSocket::errorOccurred, [](QLocalSocket::LocalSocketError error) {
            qDebug() << "Socket error:" << error;
        });
    }
    void connectToServer(const QString& pipeName) {
        socket->connectToServer("\\\\.\\pipe\\" + pipeName);
        if (!socket->waitForConnected(5000)) {
            qDebug() << "Connection failed:" << socket->errorString();
        } else {
            qDebug() << "Connected to server!";
        }
    }
    void sendMessage(const QString& message) {
        if (socket->state() == QLocalSocket::ConnectedState) {
            socket->write(message.toLocal8Bit().data());
            socket->flush();
        } else {
            qDebug() << "Socket not connected to server!";
        }
    }
private slots:
    void handleReadyRead() {
        QTextStream qout(stdout);
        QByteArray response = socket->readAll();
        qout << response << '\n';
    }
private:
    QLocalSocket *socket;
};

class ConsoleInputHandler : public QObject {
    //Q_OBJECT
public:
    explicit ConsoleInputHandler(NamedPipeClient *client, QObject *parent = nullptr)
        : QObject(parent) {
        pipeClient = client;
        stdinTimer = new QTimer();
        connect(stdinTimer, &QTimer::timeout, this, &ConsoleInputHandler::readInput);
        stdinTimer->start(100);
    }

private slots:
    void readInput() {
        QTextStream qin(stdin);
        QString input = qin.readLine();
        if (!input.isEmpty()) {
            pipeClient->sendMessage(input);
        }
        stdinTimer->start(100);
    }

private:
    QTimer *stdinTimer;
    NamedPipeClient *pipeClient;
};

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    NamedPipeClient client;
    client.connectToServer("testpipe");
    setbuf(stdin, NULL);
    ConsoleInputHandler inputHandler(&client);

    return a.exec();
}

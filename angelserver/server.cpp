#include <QtGui>
#include <QtNetwork>
#include <QDebug>
#include <QProcess>
#include <stdlib.h>
#include <QXmlQuery>
#include "server.h"

const QString KXmlFileName      = "xmlfilename";
const QString KId               = "id";
const QString KSource           = "source";
const QString KPlayer           = "player";
const QString KXqReadCommandForPlayer = "let $root:=doc($xmlfilename)//commands where $root/@player=$player return data($root/@command)";
const QString KXqReadOption     = "let $root:=doc($xmlfilename)//commands for $operation in $root/operation where $operation/@id=$id return data($operation/option)";
const QString KLinuxCommandFileName = "linux_commands.xml";
const QString KWindowsCommandFileName = "win_commands.xml";
const QString KRhythmbox        = "rhythmbox";
const QString KWinamp           = "winamp";
const QString KNowPlaying       = "nowplaying";
const QString KPlay             = "play";
const QString KSyncNow          = "syncnow";
const QString KConnect          = "connect";
const QString KResponseTemplate = "<response><status>%1</status><request>%2</request><text>%3</text></response>";

const int KStatusSuccess =  200;
const int KStatusInternalError = 500;
const int KOneSecondInMs = 1000;

Server::Server(QWidget *parent)
:   QDialog(parent), tcpServer(0), networkSession(0),
    mInternalSync(false)
{
    statusLabel = new QLabel;
    quitButton = new QPushButton(tr("Quit"));
    quitButton->setAutoDefault(false);
    mCurrentTrackName.clear();
    mProcess = new QProcess(this);
    mXmlQuery = new QXmlQuery;
    mCurrentRequest.clear();
    connect(mProcess,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(processFinished(int,QProcess::ExitStatus)));

// Populate command filename depending on the underlying platform
    QString commandsFileName;
#ifdef Q_OS_LINUX
    commandsFileName = ":/xml/linux_commands.xml";
    // TODO: Add support to get the player name dynamically
    mPlayerName = KRhythmbox;
#else Q_OS_WIN32
    commandsFileName = ":/xml/win_commands.xml";
    mPlayerName = KWinamp;
#endif

    mCommands = new QFile(commandsFileName,this);
    if(!mCommands->open(QIODevice::ReadOnly))
    {

    }

    openSession();
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(quitButton);
    buttonLayout->addStretch(1);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);
    setWindowTitle(tr("Angel Server"));
}

Server::~Server()
{
    delete mXmlQuery;
}

void Server::openSession()
{
    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen(QHostAddress::Any,1500)) {
        QMessageBox::critical(this, tr("Angel Server"),
                              tr("Unable to start the server: %1.")
                              .arg(tcpServer->errorString()));
        close();
        return;
    }

    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
            ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    statusLabel->setText(tr("The server is running on\n\nIP: %1\nport: %2\n\n"
                            "Run the Angel Client now. \n"
                            "In case of connection error, manually set IP and port and click connect in Angel Client")
                         .arg(ipAddress).arg(tcpServer->serverPort()));
}

void Server::handleNewConnection()
{
    mCommandForPlayer = commandForPlayer(mPlayerName);
    QByteArray response = "Conneted Successfully : "+mPlayerName.toLocal8Bit();

    if(mCommandForPlayer.isEmpty())
    {
        response = "No supporting player!";
    }
    mClientConnection = tcpServer->nextPendingConnection();
    connect(mClientConnection, SIGNAL(disconnected()),
            mClientConnection, SLOT(deleteLater()));

    connect(mClientConnection,SIGNAL(readyRead()),this,SLOT(handleRequest()));
    sendResponse(KStatusSuccess,response);

    // start sync timer
    startTimer(KOneSecondInMs*4);
}

void Server::handleRequest()
{
    mCurrentRequest = mClientConnection->readAll().simplified();
    qDebug()<<"requestFromClient: "<<mCurrentRequest;
    executeCommand(mCurrentRequest);
}

void Server::executeCommand(QString aRequest)
{
    QString opt = option(aRequest);
    QString commandToExecute = mCommandForPlayer+opt;
    commandToExecute = commandToExecute.simplified();
    qDebug()<<"commandToExecute:"<<commandToExecute;
    mProcess->start(commandToExecute);
}

void Server::processFinished (int exitCode,QProcess::ExitStatus exitStatus)
{
qDebug()<<__FUNCTION__;
    QString response = mProcess->readAllStandardOutput().simplified();

    // check if sycn is required
    if(mInternalSync)
    {
        mInternalSync = false;
        qDebug()<<"sync required: "<<mCurrentTrackName<<" "<<response;
        if(mCurrentTrackName != response)
        {
        mCurrentTrackName = response;
        sync();
        }

    }

    else
    {
    int stat = (0 == exitCode)?(KStatusSuccess):(KStatusInternalError);
    sendResponse(stat,response);
    }
}

void Server::readProcessOutput()
{
    qDebug()<<__FUNCTION__;
    qDebug()<<mProcess->exitStatus();
    qDebug()<<mProcess->exitCode();
}

void Server::timerEvent(QTimerEvent *event)
{
    checkIsSyncRequired();
}

void Server::checkIsSyncRequired()
{
    mInternalSync = true;
    executeCommand(KNowPlaying); // check for track title
}

void Server::sync()
{
    sendResponse(KStatusSuccess,KSyncNow);
}

QString Server::commandForPlayer(QString aPlayerName)
{
    mCommands->reset();
    QXmlQuery* xmlQuery = new QXmlQuery;
    xmlQuery->bindVariable(KXmlFileName,mCommands);

    xmlQuery->bindVariable(KPlayer,QVariant(aPlayerName));
    xmlQuery->setQuery(KXqReadCommandForPlayer);
    QString result = QString();
    if(xmlQuery->isValid())
    {
        xmlQuery->evaluateTo(&result);
    }
    delete xmlQuery;
    return result;
}

QString Server::option(QString aId)
{
    mCommands->reset();
    mXmlQuery->bindVariable(KXmlFileName,mCommands);
    mXmlQuery->bindVariable(KId,QVariant(aId));
    mXmlQuery->setQuery(KXqReadOption);
    QString result = QString();
    if(mXmlQuery->isValid())
    {
       mXmlQuery->evaluateTo(&result);
    }
    qDebug()<<result;
    return result;
}

void Server::sendResponse(int aStatus, QString aResponseText)
{
    qDebug()<<__FUNCTION__;
    QString resp = KResponseTemplate.arg(aStatus).arg(mCurrentRequest).arg(aResponseText);
    qDebug()<<resp;
    mClientConnection->write(resp.toUtf8());
}

//eof

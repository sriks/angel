#ifndef ANGELCLIENT_H
#define ANGELCLIENT_H

#include <QWidget>
#include <QAbstractSocket>
#include <QTime>
#include <QBasicTimer>
#include <QXmlQuery>
#include <QBuffer>
namespace Ui {
    class AngelClient;
}


class QTcpSocket;
class QAbstractSocket;
class AngelClient : public QWidget
{
    Q_OBJECT

public:
    explicit AngelClient(QWidget *parent = 0);
    ~AngelClient();
    QSize sizeHint();

private slots:
    void handleHostFound();
    void readServerResponse();
    void connectToServer();
    void handleError(QAbstractSocket::SocketError error);
    void playPause();
    void next();
    void prev();
    void trackDuration();
    void trackPosition();
    void showHelp();
    void sendRequest(QByteArray aRequest);
    void sliderValueChanged(int aNewValue);
    void sliderMoved(int aNewValue);
    void updateElapsedTime();
    void sync();
    QString hostAddressToConnect();

private:
    QString readResponse(QString aSourceXml,QString aResponseType);
    int timeInSecs(QString aTimeInText);
    void timerEvent(QTimerEvent *aEvent);
    void paintEvent(QPaintEvent *aPaintEvent);
    void resizeEvent(QResizeEvent *aEvent);
    void setButtonSize();

private:
    QTcpSocket* mClientSocket;
    QByteArray mCurrentRequest;
    int mTrackDurationInSec;
    QTime mTrackElapsedTime;
    bool mHasHourPart;
    bool mIsPaused;
    int mSyncTimerId;
    QBasicTimer mTrackTimer;
    QXmlQuery* mXmlQuery;
    QBuffer* mBuffer;

private:
    Ui::AngelClient *ui;
};

#endif // ANGELCLIENT_H

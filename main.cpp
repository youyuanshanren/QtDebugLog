#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QThread>
#include <thread>

#include "debug_log_ctrl.h"

void testdebug()
{
    quint32 times = 0;
    while (++times < 10000)
    {
        qDebug("qdebug %d", times);
        qWarning("qWaring %d", times);
        qCritical() << "qCritical " << times;
        qInfo() << "qInfo " << times;
        QThread::msleep(500);
    }
}

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    DebugLogCtrl::setPath(QStringLiteral("./"));
    DebugLogCtrl::setSaveLevel(0);
    DebugLogCtrl::setOutput(true);
    DebugLogCtrl::setSaveLogFile(true);
    DebugLogCtrl::setFileName(DebugLogCtrl::Name_FixedOrder,
                              QStringLiteral("test"));

    DebugLogCtrl::setFileMaxSize(10000);
    DebugLogCtrl::setMaxFileNum(5);
    DebugLogCtrl::Init();

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/qmldebug/Main.qml"_qs);
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
    engine.load(url);

    std::thread td([]() { testdebug(); });
    td.detach();

    auto err = app.exec();
    DebugLogCtrl::flush();
    return err;
}

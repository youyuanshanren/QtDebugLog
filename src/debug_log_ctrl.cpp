#include "debug_log_ctrl.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QMessageLogContext>
#include <QtCore/QThreadPool>
#include <QtCore/QTimer>
//
#include <atomic>
#include <iostream>
#include <utility>

const quint32 TIMES = 5U;
const quint32 MSLEEP_TIME = 20U;

class LogFile
{
    Q_DISABLE_COPY_MOVE(LogFile)
public:
    LogFile() = default;

    ~LogFile() { release(); }

    void setWriteImmediately(bool enable) { write_immediately = enable; }

    void setFileName(const QString& fullFileName, bool append = true)
    {
        quint32 time = 0;
        // current one thread
        while (bwrite && (time < TIMES))
        {
            ++time;
            QThread::msleep(MSLEEP_TIME);
        }

        auto* tmpfile = out_file;
        auto* tmpstream = out_stream;

        auto flag = append ? QIODevice::Append : QIODeviceBase::Truncate;

        out_file = new QFile(fullFileName);
        out_file->open(QIODevice::WriteOnly | flag);
        out_stream = new QTextStream(out_file);
        // currentFileSize = outfile->size();
        current_file_size = out_stream->pos();

        QString begin{"\r\n##########################\r\n"};
        *out_stream << begin;
        current_file_size += begin.size();
        release(tmpfile, tmpstream);
    }

    void release() { release(out_file, out_stream); }

    static void release(QFile* file, QTextStream* stream)
    {
        if (file != nullptr)
        {
            file->flush();
            file->close();
            file->deleteLater();
            delete stream;
            stream = nullptr;
            file = nullptr;
        }
    }

    void writeLog(const QString& log)
    {
        bwrite = true;
        ++num;
        *out_stream << log << "\r\n";
        current_file_size += log.size();
        if (write_immediately || (num % TIMES == 0))
        {
            out_stream->flush();
        }
        bwrite = false;
    }

    void flush()
    {
        if (out_stream != nullptr)
        {
            bwrite = true;
            out_stream->flush();
            bwrite = false;
        }
    }

    qint64 getLogSize() const { return current_file_size; }

private:
    QFile* out_file{nullptr};
    QTextStream* out_stream{nullptr};
    volatile std::atomic_bool bwrite{false};
    quint64 num{0};
    qint64 current_file_size{0};
    bool write_immediately{false};
};

qint64 getNextDateMSecond()
{
    auto datetime = QDateTime::currentDateTime();
    auto tomorrow = QDateTime(datetime.date().addDays(1), QTime(0, 0, 0));
    return datetime.msecsTo(tomorrow);
}

struct LogCtrl
{
    QString log_path;
    QString file_prefix;
    qint32 level{0};
    quint32 file_max_num{0};
    qint64 file_max_size{0};

    DebugLogCtrl::NameType name_type{DebugLogCtrl::Name_Date};
    // DebugLogCtrl::ChangeType change_type{DebugLogCtrl::Change_Date};

    LogFile file;
    QTimer timer;
    QThreadPool thread_pool;

    bool output{true};
    bool save_log{true};
    bool check_size{false};
    bool check_file_num{false};

    QList<QFileInfo> current_log_list;
    quint32 num{0};

    //==================================================
    QString getCurrentFileName()
    {
        QString date{};

        if (file_prefix.isEmpty())
        {
            file_prefix =
                QFileInfo(QCoreApplication::applicationFilePath()).fileName();
        }

        switch (name_type)
        {
            case DebugLogCtrl::Name_Fixed:
                return QString("%1.log").arg(file_prefix);

            case DebugLogCtrl::Name_FixedOrder:
            {
                getCurrentLogList();
                if (!current_log_list.empty())
                {
                    num = current_log_list.first()
                              .baseName()
                              .remove(QString("%1_").arg(file_prefix))
                              .toUInt() +
                          1;
                }

                return QString("%1_%2.log").arg(file_prefix).arg(num++);
            }

            case DebugLogCtrl::Name_Date:
                date = QDateTime::currentDateTime().toString("yyyyMMdd");
                break;

            case DebugLogCtrl::Name_DateTime:
                date = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
                break;
        }

        return QString("%1_%2.log").arg(file_prefix, date);
    }

    void timeChangeFile()
    {
        timer.stop();
        auto time = getNextDateMSecond();
        timer.start(static_cast<int>(time));
        file.setFileName(log_path + "/" + getCurrentFileName());
    }

    void getCurrentLogList()
    {
        QDir dir(log_path);
        dir.setFilter(QDir::Files);
        QStringList fifter{"*.log"};
        dir.setNameFilters(fifter);
        current_log_list = dir.entryInfoList(fifter, QDir::Files, QDir::Time);
    }

    void checkFileNum()
    {
        getCurrentLogList();
        deleteLogFile();
    }

    void deleteLogFile() const
    {
        if (current_log_list.size() > file_max_num)
        {
            QList<QFileInfo> removeList = current_log_list.sliced(file_max_num);
            for (const auto& item : removeList)
            {
                QFile::remove(item.absoluteFilePath());
            }
        }
    }

    void init()
    {
        thread_pool.setMaxThreadCount(1);

        switch (name_type)
        {
            case DebugLogCtrl::Name_Fixed:
                file.setFileName(log_path + "/" + getCurrentFileName());
                return;

            case DebugLogCtrl::Name_Date:
                QObject::connect(&timer, &QTimer::timeout, [this]() {
                    thread_pool.start([this] { timeChangeFile(); });
                });
                timeChangeFile();
                break;

            case DebugLogCtrl::Name_FixedOrder:
            case DebugLogCtrl::Name_DateTime:
                check_size = file_max_size > 0;
                file.setFileName(log_path + "/" + getCurrentFileName());
                break;
        }

        if (file_max_num > 1)
        {
            check_file_num = true;
            if (name_type == DebugLogCtrl::Name_FixedOrder)
            {
                deleteLogFile();
            }
            else
            {
                thread_pool.start([this] { checkFileNum(); });
                // checkFileNum();
            }
        }
    }

    void messageOutput(const QString& msg)
    {
        file.writeLog(msg);
        if (check_size && file.getLogSize() > file_max_size)
        {
            file.setFileName(log_path + "/" + getCurrentFileName());
            if (check_file_num)
            {
                checkFileNum();
            }
        }
    }
};

static LogCtrl gLogCtrl;

void myMessageOutput(QtMsgType type, const QMessageLogContext& context,
                     const QString& msg)
{
    if (type < gLogCtrl.level)
    {
        return;
    }

    // qSetMessagePattern format str
    auto str = qFormatLogMessage(type, context, msg);

    // remove qml fuction name
    str = str.remove("expression for ");

    // get file name
    // #ifdef QT_DEBUG
    // const auto& filename = context.file;
    // #else
    QFileInfo info(context.file);
    auto filename = info.fileName();
    // #endif

    QDateTime time = QDateTime::currentDateTime();
    QString strTime = time.toString("yyyy-MM-dd hh:mm:ss.zzz");

    QString strMsg("");
    switch (type)
    {
        // c++:qDebug, QML:console.log, console.debug
        case QtDebugMsg:
            strMsg = QString("D");
            break;

        // c++:qWarning QML:console.warn
        case QtWarningMsg:
            strMsg = QString("W");
            break;

        // c++:qCritical QML:console.error
        case QtCriticalMsg:
            strMsg = QString("C");
            break;

        // c++:qFatal, call exit QML:none
        case QtFatalMsg:
            strMsg = QString("F");
            break;

        // c++:qInfo QML:console.info
        case QtInfoMsg:
            strMsg = QString("I");
            break;

        default:
            strMsg = QString("E");
            break;
    }

    QString out =
        QString("[%1][%2][%3:%4]%5")
            .arg(strTime, strMsg, filename, QString::number(context.line), str);

    if (gLogCtrl.output)
    {
        std::cout << out.toStdString() << std::endl;
    }

    if (gLogCtrl.save_log)
    {
        gLogCtrl.thread_pool.start([out]() { gLogCtrl.messageOutput(out); });
    }
}

DebugLogCtrl::DebugLogCtrl(qint32 level, const QString& path)
    : DebugLogCtrl()
{
    gLogCtrl.level = level;
    gLogCtrl.log_path = path;
}

DebugLogCtrl::~DebugLogCtrl()
{
    flush();
}

void DebugLogCtrl::setPath(const QString& path)
{
    gLogCtrl.log_path = path;
}

void DebugLogCtrl::setSaveLevel(qint32 level)
{
    gLogCtrl.level = level;
}

void DebugLogCtrl::setOutput(bool bOut)
{
    gLogCtrl.output = bOut;
}

void DebugLogCtrl::setSaveLogFile(bool bSave)
{
    gLogCtrl.save_log = bSave;
}

void DebugLogCtrl::setFileName(NameType type, QString prefix)
{
    gLogCtrl.name_type = type;
    gLogCtrl.file_prefix = std::move(prefix);
}

void DebugLogCtrl::setFileMaxSize(qint64 bytes)
{
    gLogCtrl.file_max_size = bytes;
}

void DebugLogCtrl::setMaxFileNum(quint32 num)
{
    gLogCtrl.file_max_num = num;
}

void DebugLogCtrl::setWriteImmediately(bool enable)
{
    gLogCtrl.file.setWriteImmediately(enable);
}

void DebugLogCtrl::Init()
{
    if (QDir dir; !dir.exists(gLogCtrl.log_path))
    {
        const auto res = dir.mkpath(gLogCtrl.log_path);
        qInfo() << gLogCtrl.log_path << " mkpath " << res;
    }

    if (gLogCtrl.save_log)
    {
        gLogCtrl.init();
    }

    qInstallMessageHandler(myMessageOutput);
    qSetMessagePattern(
        "[%{threadid}][%{function}]: %{message} [%{time process}]");
}

void DebugLogCtrl::flush()
{
    gLogCtrl.file.flush();
}

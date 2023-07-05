#ifndef DEBUG_LOG_CTRL_H
#define DEBUG_LOG_CTRL_H

#include <QString>

class DebugLogCtrl
{
    Q_DISABLE_COPY_MOVE(DebugLogCtrl)
public:
    DebugLogCtrl() = default;
    DebugLogCtrl(qint32 level, const QString& path);
    ~DebugLogCtrl();

    /**
     * @brief setPath
     * @param path :log saving path
     */
    static void setPath(const QString& path);

    /**
     * @brief setSaveLevel
     * @param level : 0~4, 0:debug 1:warning 2:critica 3:fatal 4:info
     */
    static void setSaveLevel(qint32 level);

    /**
     * @brief setOutput
     * @param bOut
     */
    static void setOutput(bool bOut);

    /**
     * @brief setSaveLogFile
     * @param bSave
     */
    static void setSaveLogFile(bool bSave);

    /**
     * @brief The NameType enum
     */
    enum NameType : quint8
    {
        Name_Fixed,       // e.g install.log, Unlimited file size
        Name_FixedOrder,  // e.g install_0.log install_1.log
        Name_Date,        // e.g pes_20221124.log, Unlimited file size
        Name_DateTime,    // e.g pes_20221124_101010.log
    };

    /**
     * @brief setFileName
     * @param type =Name_Fixed, setChangeFileType, setFileMaxSize,setMaxFileNum
     *              don't work
     * @param prefix
     */
    static void setFileName(NameType type, QString prefix = "");

    /**
     * @brief setFileMaxSizeF
     * @param bytes
     */
    static void setFileMaxSize(qint64 bytes);

    /**
     * @brief setMaxFileNum
     * @param num
     */
    static void setMaxFileNum(quint32 num);

    /**
     * @brief setWriteImmediately
     * @param enable
     */
    static void setWriteImmediately(bool enable);

    /**
     * @brief Called after the above function
     */
    static void Init();

    /**
     * @brief Refresh the file immediately
     */
    static void flush();
};

#endif  // DEBUG_LOG_CTRL_H

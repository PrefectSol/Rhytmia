#ifndef OSU_CLIENT_H
#define OSU_CLIENT_H

#include <map>
#include <string>
#include <filesystem>
#include <codecvt>
#include <fstream>

#include <Windows.h>
#include <TlHelp32.h>

typedef std::map<std::string, std::map<std::string, std::string>> cfg;

#define SLASH "\\"

class OsuClient
{
public:
    enum ResultCode
    {
        success = 0,

        createToolhelp32SnapshotError,
        process32FirstError,
        findProcessError,
        openDirectoryError,

        openPidError,
        retriveExeError,
        findCfgFileError,
        openCfgFileError,

        findAppTitleError,
        mapSelectedError,

        openChatFileError,
        getBeatmapIdError,
    };

    explicit OsuClient(const cfg &config);

    ResultCode getStatus();

    bool isClosed();

private:
    const char *m_cfgExtension;

    const char *m_osuExtension;

    ResultCode m_status;

    cfg m_config;

    DWORD m_pid;
    
    std::wstring m_processName;

    std::string m_appCfg;

    std::string m_appDirectory;

    std::string m_beatmapDirectory;

    ResultCode initialize();

    ResultCode initPid();

    ResultCode initAppDirectory();

    ResultCode getUserCfgPath(std::string *path);

    ResultCode parseCfg();

protected:
};

OsuClient *OsuClientOpen(const cfg &config);

void OsuClientClose(OsuClient *client);

std::string OsuClient_GetError();

#endif // !OSU_CLIENT_H

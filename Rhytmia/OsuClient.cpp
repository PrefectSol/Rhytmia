#include "OsuClient.h"

OsuClient::ResultCode *errorCode = NULL;
std::string p_mapProcName;
std::size_t p_appNameSize;

OsuClient *OsuClientOpen(const cfg &config)
{
    OsuClient *client = new OsuClient(config);

    if (client->getStatus() != OsuClient::success)
    {
        return NULL;
    }

    return client;
}

void OsuClientClose(OsuClient *client)
{
    if (client != NULL)
    {
        delete client;
        client = nullptr;
    }
}

std::string OsuClient_GetError()
{
    return std::string("OsuClient Error: ") + std::to_string(*errorCode);
}

OsuClient::OsuClient(const cfg &config)
    : m_cfgExtension(".cfg"), m_osuExtension(".osu"), m_config(config),
    m_status(OsuClient::ResultCode()),
    m_pid(0), m_beatmapDirectory(std::string())
{
    errorCode = &m_status;
    m_status = initialize();
}

OsuClient::ResultCode OsuClient::initialize()
{
    const std::string procName = m_config["OSU"]["client_name"];
    const std::wstring wprocName(procName.begin(), procName.end());

    m_appCfg = m_config["OSU"]["client_name"] + ".cfg";
    m_processName = wprocName + L".exe";
    p_appNameSize = m_config["OSU"]["client_name"].size();

    const OsuClient::ResultCode isInitPid = initPid();
    if (isInitPid != OsuClient::ResultCode::success)
    {
        return isInitPid;
    }

    const OsuClient::ResultCode isInitAppDir = initAppDirectory();
    if (isInitAppDir != OsuClient::ResultCode::success)
    {
        return isInitAppDir;
    }

    return parseCfg();
}

OsuClient::ResultCode OsuClient::getStatus()
{
    return m_status;
}

OsuClient::ResultCode OsuClient::initPid()
{
    const HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        return OsuClient::ResultCode::createToolhelp32SnapshotError;
    }

    PROCESSENTRY32 pe32{};
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32))
    {
        CloseHandle(hProcessSnap);

        return OsuClient::ResultCode::process32FirstError;
    }

    bool processFound = false;

    do
    {
        const std::wstring processNameFromSnapshot(pe32.szExeFile);

        if (processNameFromSnapshot.find(m_processName) != std::wstring::npos)
        {
            m_pid = pe32.th32ProcessID;
            processFound = true;

            break;
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);

    if (!processFound)
    {
        return OsuClient::ResultCode::findProcessError;
    }

    return OsuClient::ResultCode::success;
}

OsuClient::ResultCode OsuClient::initAppDirectory()
{
    const HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, m_pid);

    if (!hProcess)
    {
        return OsuClient::ResultCode::openPidError;
    }

    char exeBytePath[MAX_PATH];
    DWORD bufferSize = MAX_PATH;

    if (!QueryFullProcessImageNameA(hProcess, 0, exeBytePath, &bufferSize))
    {
        CloseHandle(hProcess);

        return OsuClient::ResultCode::retriveExeError;
    }

    CloseHandle(hProcess);

    const std::string exePath = std::string(exeBytePath);
    m_appDirectory = std::filesystem::path(exePath).parent_path().string();

    return OsuClient::ResultCode::success;
}

OsuClient::ResultCode OsuClient::parseCfg()
{
    std::string userCfgPath;
    const OsuClient::ResultCode isGetUserCfgPath = getUserCfgPath(&userCfgPath);

    if (isGetUserCfgPath != OsuClient::ResultCode::success)
    {
        return isGetUserCfgPath;
    }

    std::ifstream cfgFile(userCfgPath.c_str());
    if (!cfgFile.is_open())
    {
        return OsuClient::ResultCode::openCfgFileError;
    }

    while (cfgFile)
    {
        std::string line;
        std::getline(cfgFile, line);

        if (line.find("BeatmapDirectory") != std::string::npos)
        {
            m_beatmapDirectory = line.substr(line.find("=") + 2);
        }
    }

    cfgFile.close();

    return OsuClient::ResultCode::success;
};

OsuClient::ResultCode OsuClient::getUserCfgPath(std::string *path)
{
    for (const std::filesystem::path &entry : std::filesystem::directory_iterator(m_appDirectory))
    {
        const bool isCfg = entry.extension() == m_cfgExtension;

        if (isCfg && entry.filename() != m_appCfg)
        {
            *path = entry.string();
            return OsuClient::ResultCode::success;
        }
    }

    return OsuClient::ResultCode::findCfgFileError;
}
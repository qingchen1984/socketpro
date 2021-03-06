
#include "stdafx.h"
#include <iostream>

#include "../../../../include/sqlite/usqlite_server.h"

class CMySocketProServer : public SPA::ServerSide::CSocketProServer
{

protected:
    virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
        m_h = SPA::ServerSide::CSocketProServer::DllManager::AddALibrary("ssqlite", SPA::ServerSide::Sqlite::ENABLE_GLOBAL_SQLITE_UPDATE_HOOK);
        if (m_h) {
            PSetSqliteDBGlobalConnectionString SetSqliteDBGlobalConnectionString = (PSetSqliteDBGlobalConnectionString) GetProcAddress(m_h, "SetSqliteDBGlobalConnectionString");
            SetSqliteDBGlobalConnectionString(L"usqlite.db");
        }
        return true;
    }

private:
    HINSTANCE m_h;
};

int main(int argc, char* argv[]) {
    CMySocketProServer server;
    if (!server.Run(20901)) {
        int errCode = server.GetErrorCode();
        std::cout << "Error happens with code = " << errCode << std::endl;
    }
    std::cout << "Press any key to stop the server ......" << std::endl;
    ::getchar();
}


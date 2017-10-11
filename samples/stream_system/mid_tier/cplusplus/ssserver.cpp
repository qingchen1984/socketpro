
#include "stdafx.h"
#include "ssserver.h"
#include "config.h"

std::shared_ptr<CMySQLMasterPool> CYourServer::Master;
std::shared_ptr<CMySQLSlavePool> CYourServer::Slave;

void CYourServer::StartMySQLPools() {
    assert(g_config.m_vccSlave.size());
    assert(g_config.m_nSlaveSessions);
    assert((g_config.m_nSlaveSessions % g_config.m_vccSlave.size()) == 0);

    //These case-sensitivities depends on your DB running platform and sensitivity settings.
    //All of them are false or case-insensitive by default
    CMySQLMasterPool::Cache.SetFieldNameCaseSensitive(false);
    CMySQLMasterPool::Cache.SetTableNameCaseSensitive(false);
    CMySQLMasterPool::Cache.SetDBNameCaseSensitive(false);

    CYourServer::Master.reset(new CMySQLMasterPool(g_config.m_master_default_db.c_str(), true));

    //start master pool for cache and update accessing
    bool ok = CYourServer::Master->StartSocketPool(g_config.m_ccMaster, (unsigned int) g_config.m_nMasterSessions, 1); //one thread enough

    //compute threads and sockets_per_thread
    unsigned int threads = (unsigned int) (g_config.m_nSlaveSessions / g_config.m_vccSlave.size());
    unsigned int sockets_per_thread = (unsigned int) g_config.m_vccSlave.size();
    CYourServer::Slave.reset(new CMySQLSlavePool(g_config.m_slave_default_db.c_str()));

    typedef SPA::ClientSide::CConnectionContext* PCConnectionContext;

    //prepare connection contexts for slave pool
    PCConnectionContext *ppCCs = new PCConnectionContext[threads];
    for (unsigned int t = 0; t < threads; ++t) {
        SPA::ClientSide::CConnectionContext *pcc = new SPA::ClientSide::CConnectionContext[sockets_per_thread];
        ppCCs[t] = pcc;
        for (unsigned int s = 0; s < sockets_per_thread; ++s) {
            pcc[s] = g_config.m_vccSlave[s];
        }
    }
    //start slave pool for query accessing
    ok = CYourServer::Slave->StartSocketPool(ppCCs, threads, sockets_per_thread);

    //wait until all data of cached tables are brought from backend database server to this middle server application cache
    ok = CYourServer::Master->GetAsyncHandlers()[0]->WaitAll();

    for (unsigned int t = 0; t < threads; ++t) {
        SPA::ClientSide::CConnectionContext *pcc = ppCCs[t];
        delete[]pcc;
    }
    delete []ppCCs;
}

CYourServer::CYourServer(int nParam) : CSocketProServer(nParam) {

}

bool CYourServer::OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId) {
    std::wcout << L"Ask for a service " << serviceId << L" from user " << userId << L" with password = " << password << std::endl;
    return true; //true -- ok; false -- no permission
}

bool CYourServer::OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6) {
    //amIntegrated and amMixed not supported yet
    CSocketProServer::Config::SetAuthenticationMethod(amOwn);

    SetChatGroups();

    if (!AddServices()) {
        std::cout << "Unable to register a service" << std::endl;
        return false;
    }
    return true; //true -- ok; false -- no listening server
}

bool CYourServer::AddServices() {
    bool ok = m_SSPeer.AddMe(sidStreamSystem);
    if (!ok)
        return false;
    ok = m_SSPeer.AddSlowRequest(SPA::UDB::idGetCachedTables);
    ok = m_SSPeer.AddSlowRequest(idQueryMaxMinAvgs);
    ok = m_SSPeer.AddSlowRequest(idUploadEmployees);

    return true;
}

void CYourServer::SetChatGroups() {
    bool ok = PushManager::AddAChatGroup(SPA::UDB::STREAMING_SQL_CHAT_GROUP_ID, L"Subscribe/publish for front clients");
    ok = PushManager::AddAChatGroup(SPA::UDB::CACHE_UPDATE_CHAT_GROUP_ID, L"Cache update notification from middle tier to front");
    //ok = false;
}

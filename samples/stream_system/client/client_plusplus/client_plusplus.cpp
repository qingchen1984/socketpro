
#include "stdafx.h"
#include "webasynchandler.h"

typedef SPA::CMasterPool<false, CWebAsyncHandler> CWebMasterPool;
typedef CWebMasterPool::CSlavePool CSlavePool;

typedef SPA::ClientSide::CSocketPool<CWebAsyncHandler> CMyPool;

int main(int argc, char* argv[]) {
    CConnectionContext cc;
    std::cout << "Remote host: " << std::endl;
    std::getline(std::cin, cc.Host);
    //cc.Host = "localhost";
    cc.Port = 20901;
    cc.UserId = L"SomeUserId";
    cc.Password = L"A_Password_For_SomeUserId";
    cc.EncrytionMethod = SPA::tagEncryptionMethod::TLSv1;

    CWebMasterPool master(L"sakila", false);

    //CA file is located at the directory ../socketpro/bin
    CClientSocket::SSL::SetVerifyLocation("ca.cert.pem");

    CMyPool sp(true);

    sp.DoSslServerAuthentication = [](CMyPool *sender, CClientSocket * cs)->bool {
        int errCode;
        SPA::IUcert *cert = cs->GetUCert();
        std::cout << cert->SessionInfo << std::endl;

        const char* res = cert->Verify(&errCode);

        //do ssl server certificate authentication here

        return (errCode == 0); //true -- user id and password will be sent to server
    };

    SPA::CDataSet ds;

    bool ok = sp.StartSocketPool(cc, 1, 1);
    if (!ok) {
        std::cout << "Failed in connecting to remote middle tier server, and press any key to close the application ......" << std::endl;
        ::getchar();
        return 1;
    }
    auto handler = sp.Seek();
    CDBColumnInfoArray vCol;
    ok = handler->GetCachedTables([](int res, const std::wstring & errMsg) {
        if (res) {
            std::cout << "Get initial cache with error code: " << res << ", error message: ";
            std::wcout << errMsg.c_str() << std::endl;
        }
    }, [&ds, &vCol](CDBVariantArray & vData) {
        ds.AddRows(vCol.front().DBPath.c_str(), vCol.front().TablePath.c_str(), vData);
    }, [&ds, &vCol](CDBColumnInfoArray & meta) {
        ds.AddEmptyRowset(meta);
        vCol = meta;
    });

    ok = handler->WaitAll();

    std::cout << "Press a key to shutdown the demo application ......" << std::endl;
    ::getchar();
    return 0;
}

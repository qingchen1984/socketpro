
#ifndef _UDAPARTS_ASYNC_ODBC_HANDLER_H_
#define _UDAPARTS_ASYNC_ODBC_HANDLER_H_

#include "odbc/uodbc.h"
#include "udb_client.h"

namespace SPA {
    namespace ClientSide {
        typedef CAsyncDBHandler<SPA::Odbc::sidOdbc> CAsyncDBBase;

        class CAsyncOdbc : public CAsyncDBBase {
            //no copy constructor supported
            CAsyncOdbc(const CAsyncOdbc& ao);

            //no assignment operator supported
            CAsyncOdbc& operator=(const CAsyncOdbc& ao);

        public:

            CAsyncOdbc(CClientSocket *cs = nullptr) : CAsyncDBHandler(cs) {

            }

        public:

            virtual bool ColumnPrivileges(const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *TableName, const wchar_t *ColumnName, DExecuteResult handler = DExecuteResult(), DRows row = DRows(), DRowsetHeader rh = DRowsetHeader()) {
                UINT64 index;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                m_csDB.lock();
                index = ++m_nCall;
                m_mapRowset[m_nCall] = CRowsetHandler(rh, row);
                m_csDB.unlock();
                if (!SendRequest(SPA::Odbc::idSQLColumnPrivileges, CatalogName, SchemaName, TableName, ColumnName, index, [handler, this](CAsyncResult & ar) {
                        UINT64 fail_ok;
                        int res;
                                std::wstring errMsg;
                                ar >> res >> errMsg >> fail_ok;
                                this->m_csDB.lock();
                                this->m_lastReqId = SPA::Odbc::idSQLColumnPrivileges;
                                this->m_affected = 0;
                                this->m_dbErrCode = res;
                                this->m_dbErrMsg = errMsg;
                                auto it = this->m_mapRowset.find(this->m_indexRowset);
                        if (it != this->m_mapRowset.end()) {
                            this->m_mapRowset.erase(it);
                        }
                        if (!this->m_mapRowset.size()) {
                            this->m_nCall = 0;
                        }
                        this->m_csDB.unlock();
                        if (handler) {
                            handler(*this, res, errMsg, 0, fail_ok, CDBVariant());
                        }
                    })) {
                m_csDB.lock();
                m_mapRowset.erase(index);
                m_csDB.unlock();
                return false;
            }
                return true;
            }

            virtual bool Columns(const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *TableName, const wchar_t *ColumnName, DExecuteResult handler = DExecuteResult(), DRows row = DRows(), DRowsetHeader rh = DRowsetHeader()) {
                UINT64 index;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                m_csDB.lock();
                index = ++m_nCall;
                m_mapRowset[m_nCall] = CRowsetHandler(rh, row);
                m_csDB.unlock();
                if (!SendRequest(SPA::Odbc::idSQLColumns, CatalogName, SchemaName, TableName, ColumnName, index, [handler, this](CAsyncResult & ar) {
                        UINT64 fail_ok;
                        int res;
                                std::wstring errMsg;
                                ar >> res >> errMsg >> fail_ok;
                                this->m_csDB.lock();
                                this->m_lastReqId = SPA::Odbc::idSQLColumns;
                                this->m_affected = 0;
                                this->m_dbErrCode = res;
                                this->m_dbErrMsg = errMsg;
                                auto it = this->m_mapRowset.find(this->m_indexRowset);
                        if (it != this->m_mapRowset.end()) {
                            this->m_mapRowset.erase(it);
                        }
                        if (!this->m_mapRowset.size()) {
                            this->m_nCall = 0;
                        }
                        this->m_csDB.unlock();
                        if (handler) {
                            handler(*this, res, errMsg, 0, fail_ok, CDBVariant());
                        }
                    })) {
                m_csDB.lock();
                m_mapRowset.erase(index);
                m_csDB.unlock();
                return false;
            }
                return true;
            }

            virtual bool ProcedureColumns(const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *ProcName, const wchar_t *ColumnName, DExecuteResult handler = DExecuteResult(), DRows row = DRows(), DRowsetHeader rh = DRowsetHeader()) {
                UINT64 index;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                m_csDB.lock();
                index = ++m_nCall;
                m_mapRowset[m_nCall] = CRowsetHandler(rh, row);
                m_csDB.unlock();
                if (!SendRequest(SPA::Odbc::idSQLProcedureColumns, CatalogName, SchemaName, ProcName, ColumnName, index, [handler, this](CAsyncResult & ar) {
                        UINT64 fail_ok;
                        int res;
                                std::wstring errMsg;
                                ar >> res >> errMsg >> fail_ok;
                                this->m_csDB.lock();
                                this->m_lastReqId = SPA::Odbc::idSQLProcedureColumns;
                                this->m_affected = 0;
                                this->m_dbErrCode = res;
                                this->m_dbErrMsg = errMsg;
                                auto it = this->m_mapRowset.find(this->m_indexRowset);
                        if (it != this->m_mapRowset.end()) {
                            this->m_mapRowset.erase(it);
                        }
                        if (!this->m_mapRowset.size()) {
                            this->m_nCall = 0;
                        }
                        this->m_csDB.unlock();
                        if (handler) {
                            handler(*this, res, errMsg, 0, fail_ok, CDBVariant());
                        }
                    })) {
                m_csDB.lock();
                m_mapRowset.erase(index);
                m_csDB.unlock();
                return false;
            }
                return true;
            }

            virtual bool PrimaryKeys(const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *TableName, DExecuteResult handler = DExecuteResult(), DRows row = DRows(), DRowsetHeader rh = DRowsetHeader()) {
                UINT64 index;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                m_csDB.lock();
                index = ++m_nCall;
                m_mapRowset[m_nCall] = CRowsetHandler(rh, row);
                m_csDB.unlock();
                if (!SendRequest(SPA::Odbc::idSQLPrimaryKeys, CatalogName, SchemaName, TableName, index, [handler, this](CAsyncResult & ar) {
                        UINT64 fail_ok;
                        int res;
                                std::wstring errMsg;
                                ar >> res >> errMsg >> fail_ok;
                                this->m_csDB.lock();
                                this->m_lastReqId = SPA::Odbc::idSQLPrimaryKeys;
                                this->m_affected = 0;
                                this->m_dbErrCode = res;
                                this->m_dbErrMsg = errMsg;
                                auto it = this->m_mapRowset.find(this->m_indexRowset);
                        if (it != this->m_mapRowset.end()) {
                            this->m_mapRowset.erase(it);
                        }
                        if (!this->m_mapRowset.size()) {
                            this->m_nCall = 0;
                        }
                        this->m_csDB.unlock();
                        if (handler) {
                            handler(*this, res, errMsg, 0, fail_ok, CDBVariant());
                        }
                    })) {
                m_csDB.lock();
                m_mapRowset.erase(index);
                m_csDB.unlock();
                return false;
            }
                return true;
            }

            virtual bool ForeignKeys(const wchar_t *PKCatalogName, const wchar_t *PKSchemaName, const wchar_t *PKTableName, const wchar_t *FKCatalogName, const wchar_t *FKSchemaName, const wchar_t *FKTableName, DExecuteResult handler = DExecuteResult(), DRows row = DRows(), DRowsetHeader rh = DRowsetHeader()) {
                UINT64 index;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                m_csDB.lock();
                index = ++m_nCall;
                m_mapRowset[m_nCall] = CRowsetHandler(rh, row);
                m_csDB.unlock();
                if (!SendRequest(SPA::Odbc::idSQLForeignKeys, PKCatalogName, PKSchemaName, PKTableName, FKCatalogName, FKSchemaName, FKTableName, index, [handler, this](CAsyncResult & ar) {
                        UINT64 fail_ok;
                        int res;
                                std::wstring errMsg;
                                ar >> res >> errMsg >> fail_ok;
                                this->m_csDB.lock();
                                this->m_lastReqId = SPA::Odbc::idSQLForeignKeys;
                                this->m_affected = 0;
                                this->m_dbErrCode = res;
                                this->m_dbErrMsg = errMsg;
                                auto it = this->m_mapRowset.find(this->m_indexRowset);
                        if (it != this->m_mapRowset.end()) {
                            this->m_mapRowset.erase(it);
                        }
                        if (!this->m_mapRowset.size()) {
                            this->m_nCall = 0;
                        }
                        this->m_csDB.unlock();
                        if (handler) {
                            handler(*this, res, errMsg, 0, fail_ok, CDBVariant());
                        }
                    })) {
                m_csDB.lock();
                m_mapRowset.erase(index);
                m_csDB.unlock();
                return false;
            }
                return true;
            }

            virtual bool Procedures(const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *ProcName, DExecuteResult handler = DExecuteResult(), DRows row = DRows(), DRowsetHeader rh = DRowsetHeader()) {
                UINT64 index;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                m_csDB.lock();
                index = ++m_nCall;
                m_mapRowset[m_nCall] = CRowsetHandler(rh, row);
                m_csDB.unlock();
                if (!SendRequest(SPA::Odbc::idSQLProcedures, CatalogName, SchemaName, ProcName, index, [handler, this](CAsyncResult & ar) {
                        UINT64 fail_ok;
                        int res;
                                std::wstring errMsg;
                                ar >> res >> errMsg >> fail_ok;
                                this->m_csDB.lock();
                                this->m_lastReqId = SPA::Odbc::idSQLProcedures;
                                this->m_affected = 0;
                                this->m_dbErrCode = res;
                                this->m_dbErrMsg = errMsg;
                                auto it = this->m_mapRowset.find(this->m_indexRowset);
                        if (it != this->m_mapRowset.end()) {
                            this->m_mapRowset.erase(it);
                        }
                        if (!this->m_mapRowset.size()) {
                            this->m_nCall = 0;
                        }
                        this->m_csDB.unlock();
                        if (handler) {
                            handler(*this, res, errMsg, 0, fail_ok, CDBVariant());
                        }
                    })) {
                m_csDB.lock();
                m_mapRowset.erase(index);
                m_csDB.unlock();
                return false;
            }
                return true;
            }

            virtual bool SpecialColumns(SQLSMALLINT identifierType, const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *TableName, SQLSMALLINT scope, SQLSMALLINT nullable, DExecuteResult handler = DExecuteResult(), DRows row = DRows(), DRowsetHeader rh = DRowsetHeader()) {
                UINT64 index;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                m_csDB.lock();
                index = ++m_nCall;
                m_mapRowset[m_nCall] = CRowsetHandler(rh, row);
                m_csDB.unlock();
                if (!SendRequest(SPA::Odbc::idSQLSpecialColumns, identifierType, CatalogName, SchemaName, TableName, scope, nullable, index, [handler, this](CAsyncResult & ar) {
                        UINT64 fail_ok;
                        int res;
                                std::wstring errMsg;
                                ar >> res >> errMsg >> fail_ok;
                                this->m_csDB.lock();
                                this->m_lastReqId = SPA::Odbc::idSQLSpecialColumns;
                                this->m_affected = 0;
                                this->m_dbErrCode = res;
                                this->m_dbErrMsg = errMsg;
                                auto it = this->m_mapRowset.find(this->m_indexRowset);
                        if (it != this->m_mapRowset.end()) {
                            this->m_mapRowset.erase(it);
                        }
                        if (!this->m_mapRowset.size()) {
                            this->m_nCall = 0;
                        }
                        this->m_csDB.unlock();
                        if (handler) {
                            handler(*this, res, errMsg, 0, fail_ok, CDBVariant());
                        }
                    })) {
                m_csDB.lock();
                m_mapRowset.erase(index);
                m_csDB.unlock();
                return false;
            }
                return true;
            }

            virtual bool Statistics(const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *TableName, SQLUSMALLINT unique, SQLUSMALLINT reserved, DExecuteResult handler = DExecuteResult(), DRows row = DRows(), DRowsetHeader rh = DRowsetHeader()) {
                UINT64 index;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                m_csDB.lock();
                index = ++m_nCall;
                m_mapRowset[m_nCall] = CRowsetHandler(rh, row);
                m_csDB.unlock();
                if (!SendRequest(SPA::Odbc::idSQLStatistics, CatalogName, SchemaName, TableName, unique, reserved, index, [handler, this](CAsyncResult & ar) {
                        UINT64 fail_ok;
                        int res;
                                std::wstring errMsg;
                                ar >> res >> errMsg >> fail_ok;
                                this->m_csDB.lock();
                                this->m_lastReqId = SPA::Odbc::idSQLStatistics;
                                this->m_affected = 0;
                                this->m_dbErrCode = res;
                                this->m_dbErrMsg = errMsg;
                                auto it = this->m_mapRowset.find(this->m_indexRowset);
                        if (it != this->m_mapRowset.end()) {
                            this->m_mapRowset.erase(it);
                        }
                        if (!this->m_mapRowset.size()) {
                            this->m_nCall = 0;
                        }
                        this->m_csDB.unlock();
                        if (handler) {
                            handler(*this, res, errMsg, 0, fail_ok, CDBVariant());
                        }
                    })) {
                m_csDB.lock();
                m_mapRowset.erase(index);
                m_csDB.unlock();
                return false;
            }
                return true;
            }

            virtual bool TablePrivileges(const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *TableName, DExecuteResult handler = DExecuteResult(), DRows row = DRows(), DRowsetHeader rh = DRowsetHeader()) {
                UINT64 index;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                m_csDB.lock();
                index = ++m_nCall;
                m_mapRowset[m_nCall] = CRowsetHandler(rh, row);
                m_csDB.unlock();
                if (!SendRequest(SPA::Odbc::idSQLTablePrivileges, CatalogName, SchemaName, TableName, index, [handler, this](CAsyncResult & ar) {
                        UINT64 fail_ok;
                        int res;
                                std::wstring errMsg;
                                ar >> res >> errMsg >> fail_ok;
                                this->m_csDB.lock();
                                this->m_lastReqId = SPA::Odbc::idSQLTablePrivileges;
                                this->m_affected = 0;
                                this->m_dbErrCode = res;
                                this->m_dbErrMsg = errMsg;
                                auto it = this->m_mapRowset.find(this->m_indexRowset);
                        if (it != this->m_mapRowset.end()) {
                            this->m_mapRowset.erase(it);
                        }
                        if (!this->m_mapRowset.size()) {
                            this->m_nCall = 0;
                        }
                        this->m_csDB.unlock();
                        if (handler) {
                            handler(*this, res, errMsg, 0, fail_ok, CDBVariant());
                        }
                    })) {
                m_csDB.lock();
                m_mapRowset.erase(index);
                m_csDB.unlock();
                return false;
            }
                return true;
            }

            virtual bool Tables(const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *TableName, const wchar_t *TableType, DExecuteResult handler = DExecuteResult(), DRows row = DRows(), DRowsetHeader rh = DRowsetHeader()) {
                UINT64 index;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                m_csDB.lock();
                index = ++m_nCall;
                m_mapRowset[m_nCall] = CRowsetHandler(rh, row);
                m_csDB.unlock();
                if (!SendRequest(SPA::Odbc::idSQLTables, CatalogName, SchemaName, TableName, TableType, index, [handler, this](CAsyncResult & ar) {
                        UINT64 fail_ok;
                        int res;
                                std::wstring errMsg;
                                ar >> res >> errMsg >> fail_ok;
                                this->m_csDB.lock();
                                this->m_lastReqId = SPA::Odbc::idSQLTables;
                                this->m_affected = 0;
                                this->m_dbErrCode = res;
                                this->m_dbErrMsg = errMsg;
                                auto it = this->m_mapRowset.find(this->m_indexRowset);
                        if (it != this->m_mapRowset.end()) {
                            this->m_mapRowset.erase(it);
                        }
                        if (!this->m_mapRowset.size()) {
                            this->m_nCall = 0;
                        }
                        this->m_csDB.unlock();
                        if (handler) {
                            handler(*this, res, errMsg, 0, fail_ok, CDBVariant());
                        }
                    })) {
                m_csDB.lock();
                m_mapRowset.erase(index);
                m_csDB.unlock();
                return false;
            }
                return true;
            }

            CComVariant GetInfo(SQLUSMALLINT infoType) {
                CComVariant infoValue;
                m_csDB.lock();
                if (m_mapInfo.find(infoType) != m_mapInfo.end()) {
                    infoValue = m_mapInfo[infoType];
                }
                m_csDB.unlock();
                return infoValue;
            }

        protected:

            virtual void OnResultReturned(unsigned short reqId, CUQueue &mc) {
                switch (reqId) {
                    case SPA::Odbc::idSQLGetInfo:
                        m_mapInfo.clear();
                        while (mc.GetSize()) {
                            SQLUSMALLINT infoType;
                            CComVariant infoValue;
                            mc >> infoType >> infoValue;
                            m_csDB.lock();
                            m_mapInfo[infoType] = infoValue;
                            m_csDB.unlock();
                        }
                        break;
                    default:
                        CAsyncDBBase::OnResultReturned(reqId, mc);
                        break;
                }
            }

        private:
            std::unordered_map<SQLUSMALLINT, CComVariant> m_mapInfo;
        };
    } //namespace ClientSide
} //namespace SPA

#endif
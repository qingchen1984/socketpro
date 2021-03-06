
#ifndef __UMB_CLIENT_WRAPPER_H_
#define __UMB_CLIENT_WRAPPER_H_

#include "membuffer.h"
#include "ccloader.h"
#if defined(__ANDROID__) || defined(ANDROID)
#include <boost/unordered_map.hpp>
#else
#include <unordered_map>
#endif
#include <algorithm>
#include <memory>
#include <functional>

//this may be used for debug
#define SET_CLIENT_CALL_INFO(str) SPA::ClientSide::SetLastCallInfo(str, __LINE__, __FUNCTION__)

namespace SPA {
    namespace ClientSide {
        //this may be used for debug
        void SetLastCallInfo(const char *str, int data, const char *func);

        extern CUCriticalSection g_csSpPool;

        class CAsyncServiceHandler;

        class CAsyncResult;

        typedef std::function<void(CAsyncResult&) > ResultHandler;

        const static ResultHandler NULL_RH;

        class CAsyncResult {
        private:

            CAsyncResult(CAsyncServiceHandler *pAsyncServiceHandler, unsigned short ReqId, CUQueue &q, ResultHandler & rh)
            : AsyncServiceHandler(pAsyncServiceHandler), RequestId(ReqId), UQueue(q), CurrentAsyncResultHandler(rh) {
            }

        public:

            template<class ctype >
            CUQueue& operator>>(ctype & receiver) {
                UQueue >> receiver;
                return UQueue;
            }

        public:
            CAsyncServiceHandler *AsyncServiceHandler;
            unsigned short RequestId;
            CUQueue &UQueue;
            ResultHandler &CurrentAsyncResultHandler;

        private:
            CAsyncResult(const CAsyncResult & ar);
            CAsyncResult& operator=(const CAsyncResult & ar);

            friend class CAsyncServiceHandler;
        };

        struct CConnectionContext {

            CConnectionContext()
            : Port(0),
            EncrytionMethod(NoEncryption),
            V6(false),
            Zip(false) {

            }

            CConnectionContext(const CConnectionContext &cc)
            : Host(cc.Host),
            Port(cc.Port),
            UserId(cc.UserId),
            Password(cc.Password),
            EncrytionMethod(cc.EncrytionMethod),
            V6(cc.V6),
            Zip(cc.Zip) {
            }

            CConnectionContext& operator=(const CConnectionContext &cc) {
                if (this != &cc) {
                    Host = cc.Host;
                    Port = cc.Port;
                    UserId = cc.UserId;
                    Password = cc.Password;
                    EncrytionMethod = cc.EncrytionMethod;
                    V6 = cc.V6;
                    Zip = cc.Zip;
                }
                return *this;
            }

            bool operator==(const CConnectionContext &cc) const {
                if (this == &cc)
                    return true;
                return (Host == cc.Host &&
                        Port == cc.Port &&
                        UserId == cc.UserId &&
                        Password == cc.Password &&
                        EncrytionMethod == cc.EncrytionMethod &&
                        V6 == cc.V6 &&
                        Zip == cc.Zip);
            }

            std::string Host;
            unsigned int Port;
            std::wstring UserId;
            std::wstring Password;
            tagEncryptionMethod EncrytionMethod;
            bool V6;
            bool Zip;
        };

        struct IClientQueue : public IMessageQueueBasic {
            virtual bool StartQueue(const char *qName, unsigned int ttl, bool secure = true, bool dequeueShared = false) const = 0;
            virtual bool IsDequeueEnabled() const = 0;
            virtual bool AppendTo(const IClientQueue & clientQueue) const = 0;
            virtual bool AppendTo(const USocket_Client_Handle *handles, unsigned int count) const = 0;
            virtual bool EnsureAppending(const IClientQueue & clientQueue) const = 0;
            virtual bool EnsureAppending(const USocket_Client_Handle *handles, unsigned int count) const = 0;
            virtual void EnableRoutingQueueIndex(bool enable) const = 0;
            virtual bool IsRoutingQueueIndexEnabled() const = 0;
            virtual USocket_Client_Handle GetHandle() const = 0;
        };

        class CClientSocket {
        public:
            virtual ~CClientSocket();

        protected:
            CClientSocket();

        private:
            bool Attach(CAsyncServiceHandler *p);
            void Detach(CAsyncServiceHandler *p);
            CAsyncServiceHandler *Seek(unsigned int nServiceId);
            void Set(USocket_Client_Handle h);

        private:
            CClientSocket(const CClientSocket &cs);
            CClientSocket& operator=(const CClientSocket &cs);

            class CQueueImpl : public IClientQueue {
            public:

                CQueueImpl() : m_hSocket(0), m_nQIndex(0) {
                }

                bool StartQueue(const char *qName, unsigned int ttl, bool secure = true, bool dequeueShared = false) const;
                void StopQueue(bool permanent = false);
                unsigned int GetMessagesInDequeuing() const;
                SPA::UINT64 GetMessageCount() const;
                SPA::UINT64 GetQueueSize() const;
                bool IsAvailable() const;
                bool IsSecure() const;
                const char* GetQueueFileName() const;
                const char* GetQueueName() const;
                UINT64 CancelQueuedRequests(UINT64 startIndex, UINT64 endIndex) const;
                bool IsDequeueEnabled() const;
                bool AbortJob() const;
                bool StartJob() const;
                bool EndJob() const;
                UINT64 GetJobSize() const;
                USocket_Client_Handle GetHandle() const;
                bool AppendTo(const IClientQueue &clientQueue) const;
                bool AppendTo(const USocket_Client_Handle *handles, unsigned int count) const;
                bool EnsureAppending(const IClientQueue & clientQueue) const;
                bool EnsureAppending(const USocket_Client_Handle *handles, unsigned int count) const;
                UINT64 GetLastIndex() const;
                bool IsDequeueShared() const;
                unsigned int GetTTL() const;
                UINT64 RemoveByTTL() const;
                tagQueueStatus GetQueueOpenStatus() const;
                void Reset() const;
                std::time_t GetLastMessageTime() const;
                void EnableRoutingQueueIndex(bool enable) const;
                bool IsRoutingQueueIndexEnabled() const;
                tagOptimistic GetOptimistic() const;
                void SetOptimistic(tagOptimistic optimistic) const;

            public:
                USocket_Client_Handle m_hSocket;

            private:
                unsigned int m_nQIndex;
            };
        public:

            class CPushImpl : public IPushEx {
            private:

                CPushImpl()
                : m_cs(nullptr) {
                }

            public:
                typedef std::function<void(CClientSocket*, const CMessageSender&, const SPA::UVariant&) > DOnSendUserMessage;
                DOnSendUserMessage OnSendUserMessage;

                typedef std::function<void(CClientSocket*, const CMessageSender&, const unsigned char*, unsigned int) > DOnSendUserMessageEx;
                DOnSendUserMessageEx OnSendUserMessageEx;

                typedef std::function<void(CClientSocket*, const CMessageSender&, const unsigned int*, unsigned int, const SPA::UVariant&) > DOnPublish;
                DOnPublish OnPublish;

#if defined(WIN32_64) && _MSC_VER < 1800
                //Visual C++ has implementation limitation of std::function on the number of parameters -- temporary solution
                typedef std::tr1::function<void(const CMessageSender&, const unsigned int*, unsigned int, const unsigned char*, unsigned int) > DOnPublishEx;
#else
                typedef std::function<void(CClientSocket*, const CMessageSender&, const unsigned int*, unsigned int, const unsigned char*, unsigned int) > DOnPublishEx;
#endif
                DOnPublishEx OnPublishEx;

                typedef std::function<void(CClientSocket*, const CMessageSender&, const unsigned int*, unsigned int) > DOnSubscribe;
                DOnSubscribe OnSubscribe;

                typedef std::function<void(CClientSocket*, const CMessageSender&, const unsigned int*, unsigned int) > DOnUnsubscribe;
                DOnUnsubscribe OnUnsubscribe;

            public:
                virtual bool Subscribe(const unsigned int *pChatGroupId, unsigned int count) const;
                virtual bool Publish(const UVariant& vtMessage, const unsigned int *pGroups, unsigned int ulGroupCount) const;
                virtual bool PublishEx(const unsigned char *message, unsigned int size, const unsigned int *pChatGroupId, unsigned int count) const;
                virtual bool SendUserMessage(const UVariant& vtMessage, const wchar_t *strUserId) const;
                virtual bool SendUserMessageEx(const wchar_t *userId, const unsigned char *message, unsigned int size) const;
                virtual void Unsubscribe() const;

            private:
                CPushImpl(const CPushImpl &p);
                CPushImpl& operator=(const CPushImpl &p);

            private:
                CClientSocket *m_cs;
                friend class CClientSocket;
            };

        public:
            //the version of client core library
            static const char* GetVersion();

            //persistant message queue

            struct QueueConfigure {
                static bool IsClientQueueIndexPossiblyCrashed();
                static void SetWorkDirectory(const char *dir);
                static const char* GetWorkDirectory();
                static void SetMessageQueuePassword(const char *pwd);
            };

            //openssl server certificate verification

            struct SSL {
                static bool SetVerifyLocation(const char *caFile);
                static void SetCertificateVerifyCallback(PCertificateVerifyCallback cvc);
            };

            void Close() const;
            void Shutdown(tagShutdownType st = stBoth) const;
            bool operator==(const CClientSocket &cs) const;
            void SetZip(bool zip) const;
            bool GetZip() const;
            void SetZipLevel(SPA::tagZipLevel zl) const;
            SPA::tagZipLevel GetZipLevel() const;
            unsigned int GetConnTimeout() const;
            unsigned int GetRecvTimeout() const;
            void SetRecvTimeout(unsigned int timeout) const;
            void SetConnTimeout(unsigned int timeout) const;
            void SetAutoConn(bool autoConnecting) const;
            bool GetAutoConn() const;
            bool Sendable() const;
            IUcert* GetUCert() const;
            tagConnectionState GetConnectionState() const;
            void* GetSslHandle() const;
            bool IgnoreLastRequest(unsigned short reqId) const;
            unsigned int GetRouteeCount() const;
            bool IsRouting() const;
            unsigned int GetCountOfRequestsInQueue() const;
            unsigned short GetCurrentRequestID() const;
            unsigned int GetCurrentServiceID() const;
            unsigned short GetServerPingTime() const;
            unsigned int GetCurrentResultSize() const;
            tagEncryptionMethod GetEncryptionMethod() const;
            int GetErrorCode() const;
            std::string GetErrorMsg() const;
            bool IsConnected() const;
            void SetEncryptionMethod(tagEncryptionMethod em) const;
            USocket_Client_Handle GetHandle() const;
            static CClientSocket* Seek(USocket_Client_Handle h);

            //If socket is closed, batching requests or timed out, it will return false
            bool WaitAll(unsigned int nTimeout = (~0)) const;
            bool Cancel(unsigned int requestsQueued = (~0)) const;
            bool IsRandom() const;
            unsigned int GetBytesInSendingBuffer() const;
            unsigned int GetBytesInReceivingBuffer() const;
            unsigned int GetBytesBatched() const;
            UINT64 GetBytesReceived() const;
            UINT64 GetBytesSent() const;
            void SetUID(const wchar_t *userId) const;
            std::wstring GetUID() const;
            void SetPassword(const wchar_t *password) const;
            UINT64 GetSocketNativeHandle() const;
            CPushImpl& GetPush();
            IClientQueue& GetClientQueue();
            tagOperationSystem GetPeerOs(bool *endian = nullptr) const;

            bool DoEcho() const;
            bool SetSockOpt(tagSocketOption optName, int optValue, tagSocketLevel level = slSocket) const;
            bool SetSockOptAtSvr(tagSocketOption optName, int optValue, tagSocketLevel level = slSocket) const;
            bool TurnOnZipAtSvr(bool enableZip) const;
            bool SetZipLevelAtSvr(SPA::tagZipLevel zipLevel) const;
            std::string GetPeerName(unsigned int *port) const;

            typedef std::function<void(CClientSocket*, int) > DSocketEvent;
            DSocketEvent SocketClosed;
            DSocketEvent HandShakeCompleted;
            DSocketEvent SocketConnected;

            typedef std::function<void(CClientSocket*, unsigned short) > DRequestEvent;
            DRequestEvent BaseRequestProcessed;
            DRequestEvent AllRequestsProcessed;

            typedef std::function<void(CClientSocket*, unsigned short, CUQueue &) > DRequestProcessed;
            DRequestProcessed RequestProcessed;

#if defined(WIN32_64) && _MSC_VER < 1800
            //Visual C++ has implementation limitation of std::function on the number of parameters -- temporary solution
            typedef std::tr1::function<void(CClientSocket*, const wchar_t*, const char*, unsigned int) > DExceptionFromServer;
#else
            typedef std::function<void(CClientSocket*, unsigned short, const wchar_t*, const char*, unsigned int) > DExceptionFromServer;
#endif
            DExceptionFromServer ExceptionFromServer;

        protected:
            virtual void OnSocketClosed(int nError);
            virtual void OnHandShakeCompleted(int nError);
            virtual void OnSocketConnected(int nError);
            virtual void OnRequestProcessed(unsigned short requestId, CUQueue &q);
            virtual void OnSubscribe(const CMessageSender& sender, const unsigned int *pGroup, unsigned int count);
            virtual void OnUnsubscribe(const CMessageSender& sender, const unsigned int *pGroup, unsigned int count);
            virtual void OnPublish(const CMessageSender& sender, const unsigned int *pGroup, unsigned int count, const SPA::UVariant &vtMessage);
            virtual void OnPublishEx(const CMessageSender& sender, const unsigned int *pGroup, unsigned int count, const unsigned char *pMessage, unsigned int size);
            virtual void OnSendUserMessage(const CMessageSender& sender, const SPA::UVariant &message);
            virtual void OnSendUserMessageEx(const CMessageSender& sender, const unsigned char *pMessage, unsigned int size);
            virtual void OnExceptionFromServer(unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode);
            virtual void OnBaseRequestProcessed(unsigned short requestId);
            virtual void OnAllRequestsProcessed(unsigned short lastRequestId);

        private:
            static void WINAPI OnSocketClosed(USocket_Client_Handle handler, int nError);
            static void WINAPI OnHandShakeCompleted(USocket_Client_Handle handler, int nError);
            static void WINAPI OnSocketConnected(USocket_Client_Handle handler, int nError);
            static void WINAPI OnRequestProcessed(USocket_Client_Handle handler, unsigned short requestId, unsigned int len);
            static void WINAPI OnSubscribe(USocket_Client_Handle handler, CMessageSender sender, const unsigned int *pGroup, unsigned int count);
            static void WINAPI OnUnsubscribe(USocket_Client_Handle handler, CMessageSender sender, const unsigned int *pGroup, unsigned int count);
            static void WINAPI OnBroadcastEx(USocket_Client_Handle handler, CMessageSender sender, const unsigned int *pGroup, unsigned int count, const unsigned char *pMessage, unsigned int size);
            static void WINAPI OnBroadcast(USocket_Client_Handle handler, CMessageSender sender, const unsigned int *pGroup, unsigned int count, const unsigned char *pMessage, unsigned int size);
            static void WINAPI OnPostUserMessage(USocket_Client_Handle handler, CMessageSender sender, const unsigned char *pMessage, unsigned int size);
            static void WINAPI OnPostUserMessageEx(USocket_Client_Handle handler, CMessageSender sender, const unsigned char *pMessage, unsigned int size);
            static void WINAPI OnServerException(USocket_Client_Handle handler, unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode);
            static void WINAPI OnBaseRequestProcessed(USocket_Client_Handle handler, unsigned short requestId);
            static void WINAPI OnAllRequestsProcessed(USocket_Client_Handle handler, unsigned short lastRequestId);

            CAsyncServiceHandler *GetCurrentHandler();

        private:
            USocket_Client_Handle m_hSocket;
            CPushImpl m_PushImpl;
            std::vector<CAsyncServiceHandler*> m_vHandler;
            CQueueImpl m_QueueImpl;
            CConnectionContext m_cc;
            bool m_bRandom;
            bool m_endian;
            tagOperationSystem m_os;
            unsigned int m_nCurrSvsId;
            bool m_routing;

            static CUCriticalSection m_mutex;
            static std::vector<CClientSocket*> m_vClientSocket;

            template<typename THandler, typename TCS>
            friend class CSocketPool;
            friend class CAsyncServiceHandler;
            friend class CPushImpl;
        };

        class CAsyncServiceHandler {
            SPA::CScopeUQueue m_suCallback;
            SPA::CScopeUQueue m_suBatching;
            static CUCriticalSection m_csRR;
            static void CleanQueue(CUQueue &q);

        public:
            virtual ~CAsyncServiceHandler();
            typedef std::function<bool(CAsyncServiceHandler *ash, unsigned short, CUQueue&) > DResultReturned;
            typedef std::function<void(CAsyncServiceHandler *ash, unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode) > DServerException;
            typedef std::function<void() > DCanceled;

        protected:
            CAsyncServiceHandler(unsigned int nServiceId, CClientSocket *cs = nullptr);

        private:
            CAsyncServiceHandler(const CAsyncServiceHandler&);
            CAsyncServiceHandler& operator=(const CAsyncServiceHandler&);

            struct CResultCb {

                CResultCb() {
                }

                CResultCb(const ResultHandler &rh) : AsyncResultHandler(rh) {
                }

                CResultCb(const ResultHandler &rh, const DCanceled& canceled, const DServerException &exceptionFromServer)
                : AsyncResultHandler(rh), Canceled(canceled), ExceptionFromServer(exceptionFromServer) {
                }

                CResultCb(const CResultCb &rcb)
                : AsyncResultHandler(rcb.AsyncResultHandler), Canceled(rcb.Canceled), ExceptionFromServer(rcb.ExceptionFromServer) {
                }

                CResultCb& operator=(const CResultCb &rcb) {
                    if (this != &rcb) {
                        AsyncResultHandler = rcb.AsyncResultHandler;
                        Canceled = rcb.Canceled;
                        ExceptionFromServer = rcb.ExceptionFromServer;
                    }
                    return *this;
                }

                ResultHandler AsyncResultHandler;
                DCanceled Canceled;
                DServerException ExceptionFromServer;
            };
            typedef std::pair<unsigned short, CResultCb>* PRR_PAIR;
            static std::vector<PRR_PAIR> m_vRR;
            static PRR_PAIR Reuse();
            static void Recycle(PRR_PAIR p);

        protected:
            virtual void OnResultReturned(unsigned short reqId, CUQueue &mc);
            virtual void OnExceptionFromServer(unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode);
            virtual void OnBaseRequestprocessed(unsigned short reqId);

        public:
            DResultReturned ResultReturned;
            DServerException ServerException;

        public:
            virtual bool SendRequest(unsigned short reqId, const unsigned char *pBuffer, unsigned int size, const ResultHandler &rh);
            unsigned int GetRequestsQueued();
            void ShrinkDeque();
            unsigned int GetSvsID() const;
            void SetSvsID(unsigned int serviceId);
            bool SendRequest(unsigned short reqId, const ResultHandler &rh);
            CClientSocket *GetAttachedClientSocket();
            virtual bool WaitAll(unsigned int timeOut = (~0));
            bool StartBatching();
            bool CommitBatching(bool bBatchingAtServerSide = false);
            bool AbortBatching();
            bool IsBatching();
            virtual unsigned int CleanCallbacks();
            bool IsDequeuedResult();
            void AbortDequeuedMessage();
            bool IsDequeuedMessageAborted();
            bool IsRouteeRequest();
            static void ClearResultCallbackPool(size_t remaining);
            static size_t CountResultCallbacksInPool();

            bool ProcessR0(unsigned short reqId) {
                CScopeUQueue su;
                return P(reqId, *su);
            }

            template<typename T0>
            bool ProcessR0(unsigned short reqId, const T0 &t0) {
                CScopeUQueue su;
                su << t0;
                return P(reqId, *su);
            }

            template<typename T0, typename T1>
            bool ProcessR0(unsigned short reqId, const T0 &t0, const T1 &t1) {
                CScopeUQueue su;
                su << t0 << t1;
                return P(reqId, *su);
            }

            template<typename T0, typename T1, typename T2>
            bool ProcessR0(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2) {
                CScopeUQueue su;
                su << t0 << t1 << t2;
                return P(reqId, *su);
            }

            template<typename T0, typename T1, typename T2, typename T3>
            bool ProcessR0(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3;
                return P(reqId, *su);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4>
            bool ProcessR0(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4;
                return P(reqId, *su);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
            bool ProcessR0(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5;
                return P(reqId, *su);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
            bool ProcessR0(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6;
                return P(reqId, *su);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
            bool ProcessR0(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7;
                return P(reqId, *su);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
            bool ProcessR0(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
                return P(reqId, *su);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
            bool ProcessR0(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, const T9 &t9) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
                return P(reqId, *su);
            }

            //ProcessR1

            template<typename R0>
            bool ProcessR1(unsigned short reqId, R0 &r0) {
                CScopeUQueue su;
                return P(reqId, *su, r0);
            }

            template<typename T0, typename R0>
            bool ProcessR1(unsigned short reqId, const T0 &t0, R0 &r0) {
                CScopeUQueue su;
                su << t0;
                return P(reqId, *su, r0);
            }

            template<typename T0, typename T1, typename R0>
            bool ProcessR1(unsigned short reqId, const T0 &t0, const T1 &t1, R0 &r0) {
                CScopeUQueue su;
                su << t0 << t1;
                return P(reqId, *su, r0);
            }

            template<typename T0, typename T1, typename T2, typename R0>
            bool ProcessR1(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, R0 &r0) {
                CScopeUQueue su;
                su << t0 << t1 << t2;
                return P(reqId, *su, r0);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename R0>
            bool ProcessR1(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, R0 &r0) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3;
                return P(reqId, *su, r0);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename R0>
            bool ProcessR1(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, R0 &r0) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4;
                return P(reqId, *su, r0);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename R0>
            bool ProcessR1(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, R0 &r0) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5;
                return P(reqId, *su, r0);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename R0>
            bool ProcessR1(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, R0 &r0) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6;
                return P(reqId, *su, r0);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename R0>
            bool ProcessR1(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, R0 &r0) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7;
                return P(reqId, *su, r0);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename R0>
            bool ProcessR1(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, R0 &r0) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
                return P(reqId, *su, r0);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename R0>
            bool ProcessR1(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, const T9 &t9, R0 &r0) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
                return P(reqId, *su, r0);
            }

            //ProcessR2

            template<typename R0, typename R1>
            bool ProcessR2(unsigned short reqId, R0 &r0, R1 &r1) {
                CScopeUQueue su;
                return P(reqId, *su, r0, r1);
            }

            template<typename T0, typename R0, typename R1>
            bool ProcessR2(unsigned short reqId, const T0 &t0, R0 &r0, R1 &r1) {
                CScopeUQueue su;
                su << t0;
                return P(reqId, *su, r0, r1);
            }

            template<typename T0, typename T1, typename R0, typename R1>
            bool ProcessR2(unsigned short reqId, const T0 &t0, const T1 &t1, R0 &r0, R1 &r1) {
                CScopeUQueue su;
                su << t0 << t1;
                return P(reqId, *su, r0, r1);
            }

            template<typename T0, typename T1, typename T2, typename R0, typename R1>
            bool ProcessR2(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, R0 &r0, R1 &r1) {
                CScopeUQueue su;
                su << t0 << t1 << t2;
                return P(reqId, *su, r0, r1);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename R0, typename R1>
            bool ProcessR2(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, R0 &r0, R1 &r1) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3;
                return P(reqId, *su, r0, r1);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename R0, typename R1>
            bool ProcessR2(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, R0 &r0, R1 &r1) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4;
                return P(reqId, *su, r0, r1);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename R0, typename R1>
            bool ProcessR2(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, R0 &r0, R1 &r1) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5;
                return P(reqId, *su, r0, r1);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename R0, typename R1>
            bool ProcessR2(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, R0 &r0, R1 &r1) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6;
                return P(reqId, *su, r0, r1);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename R0, typename R1>
            bool ProcessR2(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, R0 &r0, R1 &r1) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7;
                return P(reqId, *su, r0, r1);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename R0, typename R1>
            bool ProcessR2(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, R0 &r0, R1 &r1) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
                return P(reqId, *su, r0, r1);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename R0, typename R1>
            bool ProcessR2(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, const T9 &t9, R0 &r0, R1 &r1) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
                return P(reqId, *su, r0, r1);
            }

            //ProcessR3

            template<typename R0, typename R1, typename R2>
            bool ProcessR3(unsigned short reqId, R0 &r0, R1 &r1, R2 &r2) {
                CScopeUQueue su;
                return P(reqId, *su, r0, r1, r2);
            }

            template<typename T0, typename R0, typename R1, typename R2>
            bool ProcessR3(unsigned short reqId, const T0 &t0, R0 &r0, R1 &r1, R2 &r2) {
                CScopeUQueue su;
                su << t0;
                return P(reqId, *su, r0, r1, r2);
            }

            template<typename T0, typename T1, typename R0, typename R1, typename R2>
            bool ProcessR3(unsigned short reqId, const T0 &t0, const T1 &t1, R0 &r0, R1 &r1, R2 &r2) {
                CScopeUQueue su;
                su << t0 << t1;
                return P(reqId, *su, r0, r1, r2);
            }

            template<typename T0, typename T1, typename T2, typename R0, typename R1, typename R2>
            bool ProcessR3(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, R0 &r0, R1 &r1, R2 &r2) {
                CScopeUQueue su;
                su << t0 << t1 << t2;
                return P(reqId, *su, r0, r1, r2);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename R0, typename R1, typename R2>
            bool ProcessR3(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, R0 &r0, R1 &r1, R2 &r2) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3;
                return P(reqId, *su, r0, r1, r2);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename R0, typename R1, typename R2>
            bool ProcessR3(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, R0 &r0, R1 &r1, R2 &r2) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4;
                return P(reqId, *su, r0, r1, r2);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename R0, typename R1, typename R2>
            bool ProcessR3(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, R0 &r0, R1 &r1, R2 &r2) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5;
                return P(reqId, *su, r0, r1, r2);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename R0, typename R1, typename R2>
            bool ProcessR3(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, R0 &r0, R1 &r1, R2 &r2) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6;
                return P(reqId, *su, r0, r1, r2);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename R0, typename R1, typename R2>
            bool ProcessR3(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, R0 &r0, R1 &r1, R2 &r2) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7;
                return P(reqId, *su, r0, r1, r2);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename R0, typename R1, typename R2>
            bool ProcessR3(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, R0 &r0, R1 &r1, R2 &r2) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
                return P(reqId, *su, r0, r1, r2);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename R0, typename R1, typename R2>
            bool ProcessR3(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, const T9 &t9, R0 &r0, R1 &r1, R2 &r2) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
                return P(reqId, *su, r0, r1, r2);
            }

            //ProcessR4

            template<typename R0, typename R1, typename R2, typename R3>
            bool ProcessR4(unsigned short reqId, R0 &r0, R1 &r1, R2 &r2, R3 &r3) {
                CScopeUQueue su;
                return P(reqId, *su, r0, r1, r2, r3);
            }

            template<typename T0, typename R0, typename R1, typename R2, typename R3>
            bool ProcessR4(unsigned short reqId, const T0 &t0, R0 &r0, R1 &r1, R2 &r2, R3 &r3) {
                CScopeUQueue su;
                su << t0;
                return P(reqId, *su, r0, r1, r2, r3);
            }

            template<typename T0, typename T1, typename R0, typename R1, typename R2, typename R3>
            bool ProcessR4(unsigned short reqId, const T0 &t0, const T1 &t1, R0 &r0, R1 &r1, R2 &r2, R3 &r3) {
                CScopeUQueue su;
                su << t0 << t1;
                return P(reqId, *su, r0, r1, r2, r3);
            }

            template<typename T0, typename T1, typename T2, typename R0, typename R1, typename R2, typename R3>
            bool ProcessR4(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, R0 &r0, R1 &r1, R2 &r2, R3 &r3) {
                CScopeUQueue su;
                su << t0 << t1 << t2;
                return P(reqId, *su, r0, r1, r2, r3);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename R0, typename R1, typename R2, typename R3>
            bool ProcessR4(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, R0 &r0, R1 &r1, R2 &r2, R3 &r3) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3;
                return P(reqId, *su, r0, r1, r2, r3);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename R0, typename R1, typename R2, typename R3>
            bool ProcessR4(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, R0 &r0, R1 &r1, R2 &r2, R3 &r3) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4;
                return P(reqId, *su, r0, r1, r2, r3);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename R0, typename R1, typename R2, typename R3>
            bool ProcessR4(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, R0 &r0, R1 &r1, R2 &r2, R3 &r3) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5;
                return P(reqId, *su, r0, r1, r2, r3);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename R0, typename R1, typename R2, typename R3>
            bool ProcessR4(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, R0 &r0, R1 &r1, R2 &r2, R3 &r3) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6;
                return P(reqId, *su, r0, r1, r2, r3);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename R0, typename R1, typename R2, typename R3>
            bool ProcessR4(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, R0 &r0, R1 &r1, R2 &r2, R3 &r3) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7;
                return P(reqId, *su, r0, r1, r2, r3);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename R0, typename R1, typename R2, typename R3>
            bool ProcessR4(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, R0 &r0, R1 &r1, R2 &r2, R3 &r3) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
                return P(reqId, *su, r0, r1, r2, r3);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename R0, typename R1, typename R2, typename R3>
            bool ProcessR4(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, const T9 &t9, R0 &r0, R1 &r1, R2 &r2, R3 &r3) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
                return P(reqId, *su, r0, r1, r2, r3);
            }

            //ProcessR5

            template<typename R0, typename R1, typename R2, typename R3, typename R4>
            bool ProcessR5(unsigned short reqId, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4) {
                CScopeUQueue su;
                return P(reqId, *su, r0, r1, r2, r3, r4);
            }

            template<typename T0, typename R0, typename R1, typename R2, typename R3, typename R4>
            bool ProcessR5(unsigned short reqId, const T0 &t0, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4) {
                CScopeUQueue su;
                su << t0;
                return P(reqId, *su, r0, r1, r2, r3, r4);
            }

            template<typename T0, typename T1, typename R0, typename R1, typename R2, typename R3, typename R4>
            bool ProcessR5(unsigned short reqId, const T0 &t0, const T1 &t1, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4) {
                CScopeUQueue su;
                su << t0 << t1;
                return P(reqId, *su, r0, r1, r2, r3, r4);
            }

            template<typename T0, typename T1, typename T2, typename R0, typename R1, typename R2, typename R3, typename R4>
            bool ProcessR5(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4) {
                CScopeUQueue su;
                su << t0 << t1 << t2;
                return P(reqId, *su, r0, r1, r2, r3, r4);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename R0, typename R1, typename R2, typename R3, typename R4>
            bool ProcessR5(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3;
                return P(reqId, *su, r0, r1, r2, r3, r4);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename R0, typename R1, typename R2, typename R3, typename R4>
            bool ProcessR5(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4;
                return P(reqId, *su, r0, r1, r2, r3, r4);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename R0, typename R1, typename R2, typename R3, typename R4>
            bool ProcessR5(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5;
                return P(reqId, *su, r0, r1, r2, r3, r4);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename R0, typename R1, typename R2, typename R3, typename R4>
            bool ProcessR5(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6;
                return P(reqId, *su, r0, r1, r2, r3, r4);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename R0, typename R1, typename R2, typename R3, typename R4>
            bool ProcessR5(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7;
                return P(reqId, *su, r0, r1, r2, r3, r4);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename R0, typename R1, typename R2, typename R3, typename R4>
            bool ProcessR5(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
                return P(reqId, *su, r0, r1, r2, r3, r4);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename R0, typename R1, typename R2, typename R3, typename R4>
            bool ProcessR5(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7& t7, const T8 &t8, const T9 &t9, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4) {
                CScopeUQueue su;
                su << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
                return P(reqId, *su, r0, r1, r2, r3, r4);
            }

            template<typename T0>
            bool SendRequest(unsigned short reqId, const T0 &t0, const ResultHandler &rh) {
                CScopeUQueue sb;
                sb << t0;
                return SendRequest(reqId, sb->GetBuffer(), sb->GetSize(), rh);
            }

            template<typename T0, typename T1>
            bool SendRequest(unsigned short reqId, const T0 &t0, const T1 &t1, const ResultHandler &rh) {
                CScopeUQueue sb;
                sb << t0 << t1;
                return SendRequest(reqId, sb->GetBuffer(), sb->GetSize(), rh);
            }

            template<typename T0, typename T1, typename T2>
            bool SendRequest(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const ResultHandler &rh) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2;
                return SendRequest(reqId, sb->GetBuffer(), sb->GetSize(), rh);
            }

            template<typename T0, typename T1, typename T2, typename T3>
            bool SendRequest(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const ResultHandler &rh) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3;
                return SendRequest(reqId, sb->GetBuffer(), sb->GetSize(), rh);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4>
            bool SendRequest(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const ResultHandler &rh) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4;
                return SendRequest(reqId, sb->GetBuffer(), sb->GetSize(), rh);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
            bool SendRequest(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const ResultHandler &rh) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5;
                return SendRequest(reqId, sb->GetBuffer(), sb->GetSize(), rh);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
            bool SendRequest(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const ResultHandler &rh) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6;
                return SendRequest(reqId, sb->GetBuffer(), sb->GetSize(), rh);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
            bool SendRequest(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const ResultHandler &rh) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7;
                return SendRequest(reqId, sb->GetBuffer(), sb->GetSize(), rh);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
            bool SendRequest(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, const ResultHandler &rh) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
                return SendRequest(reqId, sb->GetBuffer(), sb->GetSize(), rh);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
            bool SendRequest(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, const T9 &t9, const ResultHandler &rh) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
                return SendRequest(reqId, sb->GetBuffer(), sb->GetSize(), rh);
            }

            virtual bool SendRequestFuture(unsigned short reqId, const unsigned char *pBuffer, unsigned int size, const ResultHandler &rh, const DCanceled &canceled, const DServerException &serverException) {
                USocket_Client_Handle h = GetClientSocketHandle();
                CAutoLock alSend(m_csSend);
                if (rh || canceled || serverException) {
                    PRR_PAIR p = Reuse();
                    if (p) {
                        p->first = reqId;
                        p->second.AsyncResultHandler = rh;
                        p->second.Canceled = canceled;
                        p->second.ExceptionFromServer = serverException;
                    } else {
                        p = new std::pair<unsigned short, CResultCb>(reqId, CResultCb(rh, canceled, serverException));
                    }
                    CAutoLock al(m_cs);
                    bool batching = ClientCoreLoader.IsBatching(h);
                    if (batching)
                        m_vBatching << p;
                    else
                        m_vCallback << p;
                }
                return ClientCoreLoader.SendRequest(h, reqId, pBuffer, size);
            }

#if defined(_FUTURE_) || defined(_GLIBCXX_FUTURE)

            std::future<void> async(unsigned short reqId, const unsigned char *pBuffer, unsigned int size) {
                std::shared_ptr<std::promise<void> > prom(new std::promise<void>, [](std::promise<void> *p) {
                    delete p;
                });
                DCanceled canceled = [prom, reqId]() {
                    try {
                        prom->set_exception(std::make_exception_ptr(CUException("Task aborted", __FILE__, reqId, __FUNCTION__, MB_REQUEST_ABORTED)));
                    } catch (...) {
                    }
                };
                DServerException se = [prom](CAsyncServiceHandler *ash, unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode) {
#if defined(__ANDROID__) || defined(ANDROID)
                    std::string message = Utilities::ToUTF8(errMessage, ::wcslen(errMessage));
                    try {
                        prom->set_exception(std::make_exception_ptr(CUException(message.c_str(), __FILE__, requestId, __FUNCTION__, errCode)));
#else
                    CScopeUQueue sq;
                    Utilities::ToUTF8(errMessage, ::wcslen(errMessage), *sq);
                    try {
                        prom->set_exception(std::make_exception_ptr(CUException((const char*) sq->GetBuffer(), __FILE__, requestId, __FUNCTION__, errCode)));
#endif
                    } catch (...) {
                    }
                };
                if (!SendRequestFuture(reqId, pBuffer, size, [prom](CAsyncResult & ar) {
                        prom->set_value();
                    }, canceled, se)) {
                throw CUException(GetAttachedClientSocket()->GetErrorMsg().c_str(), __FILE__, reqId, __FUNCTION__, GetAttachedClientSocket()->GetErrorCode());
            }
                return prom->get_future();
            }

            std::future<void> async(unsigned short reqId) {
                return async(reqId, (const unsigned char *) nullptr, (unsigned int) 0);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
            std::future<void> async(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, const T9 &t9) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
                return async(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
            std::future<void> async(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
                return async(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
            std::future<void> async(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7;
                return async(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
            std::future<void> async(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6;
                return async(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
            std::future<void> async(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5;
                return async(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4>
            std::future<void> async(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4;
                return async(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0, typename T1, typename T2, typename T3>
            std::future<void> async(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3;
                return async(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0, typename T1, typename T2>
            std::future<void> async(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2;
                return async(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0, typename T1>
            std::future<void> async(unsigned short reqId, const T0 &t0, const T1 &t1) {
                CScopeUQueue sb;
                sb << t0 << t1;
                return async(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0>
            std::future<void> async(unsigned short reqId, const T0 &t0) {
                CScopeUQueue sb;
                sb << t0;
                return async(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename R>
            std::future<R> async(unsigned short reqId, const unsigned char *pBuffer, unsigned int size) {
                std::shared_ptr<std::promise<R> > prom(new std::promise<R>, [](std::promise<R> *p) {
                    delete p;
                });
                DCanceled canceled = [prom, reqId]() {
                    try {
                        prom->set_exception(std::make_exception_ptr(CUException("Task aborted", __FILE__, reqId, __FUNCTION__, MB_REQUEST_ABORTED)));
                    } catch (...) {
                    }
                };
                DServerException se = [prom](CAsyncServiceHandler *ash, unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode) {
#if defined(__ANDROID__) || defined(ANDROID)
                    std::string message = Utilities::ToUTF8(errMessage, ::wcslen(errMessage));
                    try {
                        prom->set_exception(std::make_exception_ptr(CUException(message.c_str(), __FILE__, requestId, __FUNCTION__, errCode)));
#else
                    CScopeUQueue sq;
                    Utilities::ToUTF8(errMessage, ::wcslen(errMessage), *sq);
                    try {
                        prom->set_exception(std::make_exception_ptr(CUException((const char*) sq->GetBuffer(), __FILE__, requestId, __FUNCTION__, errCode)));
#endif
                    } catch (...) {
                    }
                };
                if (!SendRequestFuture(reqId, pBuffer, size, [prom](CAsyncResult & ar) {
                        try {
                            R r;
                            ar >> r;
                            prom->set_value(r);
                        } catch (...) {
                            prom->set_exception(std::current_exception());
                        }
                    }, canceled, se)) {
                throw CUException(GetAttachedClientSocket()->GetErrorMsg().c_str(), __FILE__, reqId, __FUNCTION__, GetAttachedClientSocket()->GetErrorCode());
            }
                return prom->get_future();
            }

            template<typename R>
            std::future<R> async(unsigned short reqId) {
                return async<R>(reqId, (const unsigned char *) nullptr, (unsigned int) 0);
            }

            template<typename R, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
            std::future<R> async(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, const T9 &t9) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
                return async<R>(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename R, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
            std::future<R> async(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
                return async<R>(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename R, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
            std::future<R> async(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7;
                return async<R>(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename R, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
            std::future<R> async(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6;
                return async<R>(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename R, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
            std::future<R> async(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5;
                return async<R>(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename R, typename T0, typename T1, typename T2, typename T3, typename T4>
            std::future<R> async(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4;
                return async<R>(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename R, typename T0, typename T1, typename T2, typename T3>
            std::future<R> async(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3;
                return async<R>(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename R, typename T0, typename T1, typename T2>
            std::future<R> async(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2) {
                CScopeUQueue sb;
                sb << t0 << t1 << t2;
                return async<R>(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename R, typename T0, typename T1>
            std::future<R> async(unsigned short reqId, const T0 &t0, const T1 &t1) {
                CScopeUQueue sb;
                sb << t0 << t1;
                return async<R>(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename R, typename T0>
            std::future<R> async(unsigned short reqId, const T0 &t0) {
                CScopeUQueue sb;
                sb << t0;
                return async<R>(reqId, sb->GetBuffer(), sb->GetSize());
            }
#endif
        private:
            //The two following functions may be public in the future
            bool Attach(CClientSocket *cs);
            void Detach();

            USocket_Client_Handle GetClientSocketHandle() const;

            bool P(unsigned short reqId, const CUQueue &qSender) {
                if (!SendRequest(reqId, qSender.GetBuffer(), qSender.GetSize(), [](CAsyncResult & ar) {
                    })) {
                return false;
            }
                return WaitAll();
            }

            template<typename R0>
            bool P(unsigned short reqId, const CUQueue &qSender, R0 &r0) {
                if (!SendRequest(reqId, qSender.GetBuffer(), qSender.GetSize(), [&r0](CAsyncResult & ar) {
                        ar >> r0;
                    })) {
                return false;
            }
                return WaitAll();
            }

            template<typename R0, typename R1>
            bool P(unsigned short reqId, const CUQueue &qSender, R0 &r0, R1 &r1) {
                if (!SendRequest(reqId, qSender.GetBuffer(), qSender.GetSize(), [&r0, &r1](CAsyncResult & ar) {
                        ar >> r0 >> r1;
                    })) {
                return false;
            }
                return WaitAll();
            }

            template<typename R0, typename R1, typename R2>
            bool P(unsigned short reqId, const CUQueue &qSender, R0 &r0, R1 &r1, R2 &r2) {
                if (!SendRequest(reqId, qSender.GetBuffer(), qSender.GetSize(), [&r0, &r1, &r2](CAsyncResult & ar) {
                        ar >> r0 >> r1 >> r2;
                    })) {
                return false;
            }
                return WaitAll();
            }

            template<typename R0, typename R1, typename R2, typename R3>
            bool P(unsigned short reqId, const CUQueue &qSender, R0 &r0, R1 &r1, R2 &r2, R3 &r3) {
                if (!SendRequest(reqId, qSender.GetBuffer(), qSender.GetSize(), [&r0, &r1, &r2, &r3](CAsyncResult & ar) {
                        ar >> r0 >> r1 >> r2 >> r3;
                    })) {
                return false;
            }
                return WaitAll();
            }

            template<typename R0, typename R1, typename R2, typename R3, typename R4>
            bool P(unsigned short reqId, const CUQueue &qSender, R0 &r0, R1 &r1, R2 &r2, R3 &r3, R4 &r4) {
                if (!SendRequest(reqId, qSender.GetBuffer(), qSender.GetSize(), [&r0, &r1, &r2, &r3, &r4](CAsyncResult & ar) {
                        ar >> r0 >> r1 >> r2 >> r3 >> r4;
                    })) {
                return false;
            }
                return WaitAll();
            }

            bool GetAsyncResultHandler(unsigned short usReqId, PRR_PAIR &p);
            void OnRR(unsigned short reqId, CUQueue &mc);
            void OnSE(unsigned short requestId, const wchar_t *errMessage, const char* errWhere, unsigned int errCode);
            void SetNULL();
            void EraseBack(unsigned int count);
            void AppendTo(CAsyncServiceHandler &from);

        protected:
            virtual bool SendRouteeResult(const unsigned char *buffer, unsigned int len, unsigned short reqId = 0);
            bool SendRouteeResult(unsigned short reqId = 0);
            bool SendRouteeResult(const CUQueue &mc, unsigned short reqId = 0);
            bool SendRouteeResult(const CScopeUQueue &sb, unsigned short reqId = 0);

            template <class ctype0>
            bool SendRouteeResult(const ctype0& data0, unsigned short usRequestID = 0) {
                CScopeUQueue sb;
                sb << data0;
                return SendRouteeResult(sb->GetBuffer(), sb->GetSize(), usRequestID);
            }

            template<class ctype0, class ctype1>
            bool SendRouteeResult(const ctype0& data0, const ctype1& data1, unsigned short usRequestID = 0) {
                CScopeUQueue sb;
                sb << data0 << data1;
                return SendRouteeResult(sb->GetBuffer(), sb->GetSize(), usRequestID);
            }

            template<class ctype0, class ctype1, class ctype2>
            bool SendRouteeResult(const ctype0& data0, const ctype1& data1, const ctype1& data2, unsigned short usRequestID = 0) {
                CScopeUQueue sb;
                sb << data0 << data1 << data2;
                return SendRouteeResult(sb->GetBuffer(), sb->GetSize(), usRequestID);
            }

            template<class ctype0, class ctype1, class ctype2, class ctype3>
            bool SendRouteeResult(const ctype0& data0, const ctype1& data1, const ctype2& data2, const ctype3& data3, unsigned short usRequestID = 0) {
                CScopeUQueue sb;
                sb << data0 << data1 << data2 << data3;
                return SendRouteeResult(sb->GetBuffer(), sb->GetSize(), usRequestID);
            }

            template<class ctype0, class ctype1, class ctype2, class ctype3, class ctype4>
            bool SendRouteeResult(const ctype0& data0, const ctype1& data1, const ctype2& data2, const ctype3& data3, const ctype4& data4, unsigned short usRequestID = 0) {
                CScopeUQueue sb;
                sb << data0 << data1 << data2 << data3 << data4;
                return SendRouteeResult(sb->GetBuffer(), sb->GetSize(), usRequestID);
            }

        private:
            CUCriticalSection m_cs;
            CUQueue &m_vCallback;
            CUQueue &m_vBatching;
            unsigned int m_nServiceId;
            CClientSocket *m_pClientSocket;
            CUCriticalSection m_csSend;
            friend class CClientSocket;
            template<typename THandler, typename TCS>
            friend class CSocketPool; // unbound friend class
        };

        class CStreamHelper {
        public:
            static const UINT64 INVALID_SIZE = 0xffffffffffffffff;

            CStreamHelper(CAsyncServiceHandler& ash)
            : m_ash(ash), m_br(nullptr), m_bw(nullptr), m_nDownloadFileSize(~0), m_nPos(0) {
                m_ash.ResultReturned = [this](CAsyncServiceHandler *ash, unsigned short reqId, CUQueue & q) -> bool {
                    bool processed = false;
                    switch (reqId) {
                        case CStreamSerializationHelper::idReadDataFromServerToClient:
                            if (q.GetSize() > 0) {
                                if (this->m_bw)
                                    CStreamSerializationHelper::Write(*(this->m_bw), q);
                                m_nPos += q.GetSize();
                                if (this->Progress && this->m_bw)
                                    this->Progress(this, m_nPos);
                                q.SetSize(0);
                            }
                            processed = true;
                            break;
                        default:
                            break;
                    }
                    return processed;
                };
            }
            typedef std::function<void(const CStreamHelper*, UINT64 pos) > DProgress;
            DProgress Progress;

        public:

            CAsyncServiceHandler& GetAsyncServiceHandler() {
                return m_ash;
            }

            void Reset() {
                m_bw = nullptr;
                CAutoLock al(m_cs);
                m_br = nullptr;
            }

            UINT64 GetDownloadingStreamSize() const {
                return m_nDownloadFileSize;
            }

            std::wstring DownloadFile(std::ostream &bw, const std::wstring& RemoteFilePath) {
                if (!bw.good())
                    return L"Bad stream";
                {
                    CAutoLock al(m_cs);
                    if (m_br || m_br)
                        return L"A stream during transaction";
                }
                m_bw = &bw;
                std::wstring res;
                m_nDownloadFileSize = CStreamHelper::INVALID_SIZE;
                m_nPos = 0;
                bool ok = (m_ash.SendRequest(CStreamSerializationHelper::idStartDownloading, RemoteFilePath, [this, &res](CAsyncResult & ar) {
                    ar >> this->m_nDownloadFileSize >> res;
                    if (this->Progress && this->m_nDownloadFileSize != CStreamHelper::INVALID_SIZE && res.size() == 0 && this->m_bw)
                            this->Progress(this, m_nPos);
                    }) && m_ash.WaitAll());
                if (res.size() > 0) {
                    m_bw = nullptr;
                    return res;
                }
                if (!ok && !m_ash.GetAttachedClientSocket()->Sendable()) {
                    m_bw = nullptr;
                    std::string s = m_ash.GetAttachedClientSocket()->GetErrorMsg();
#if defined(__ANDROID__) || defined(ANDROID)
                    return Utilities::ToWide(s.c_str(), s.size());
#else
                    CScopeUQueue su;
                    Utilities::ToWide(s.c_str(), s.size(), *su);
                    return (const wchar_t*)su->GetBuffer();
#endif
                }
                if (!m_ash.SendRequest(CStreamSerializationHelper::idDownloadCompleted, [this](CAsyncResult & ar) {
                        if (this->Progress && m_bw)
                            this->Progress(this, m_nPos);
                        m_bw = nullptr;
                    })) {
                m_bw = nullptr;
                std::string s = m_ash.GetAttachedClientSocket()->GetErrorMsg();
#if defined(__ANDROID__) || defined(ANDROID)
                return Utilities::ToWide(s.c_str(), s.size());
#else
                CScopeUQueue su;
                Utilities::ToWide(s.c_str(), s.size(), *su);
                return (const wchar_t*)su->GetBuffer();
#endif
            }
                return res;
            }

            std::wstring UploadFile(std::istream &br, const std::wstring& RemoteFilePath) {
                bool ok;
                if (!br.good())
                    return L"Bad stream";
                {
                    CAutoLock al(m_cs);
                    if (m_br || m_br)
                        return L"A stream during transaction";
                }
                std::wstring res;
                ok = (m_ash.SendRequest(CStreamSerializationHelper::idStartUploading, RemoteFilePath, [&res](CAsyncResult & ar) {
                    ar >> res;
                }) && m_ash.WaitAll());
                if (res.size() > 0)
                    return res;
                if (!ok && !m_ash.GetAttachedClientSocket()->Sendable()) {
                    std::string s = m_ash.GetAttachedClientSocket()->GetErrorMsg();
#if defined(__ANDROID__) || defined(ANDROID)
                    return Utilities::ToWide(s.c_str(), s.size());
#else
                    CScopeUQueue su;
                    Utilities::ToWide(s.c_str(), s.size(), *su);
                    return (const wchar_t*)su->GetBuffer();
#endif
                }
                {
                    CAutoLock al(m_cs);
                    if (m_br)
                        return L"A stream during transaction";
                    m_nPos = 0;
                    m_br = &br;
                    if (Progress)
                        Progress(this, m_nPos);
                    if (SendDataFromClientToServer(br) == 0) {
                        if (!m_ash.SendRequest(CStreamSerializationHelper::idUploadCompleted, [this](CAsyncResult & ar) {
                                CAutoLock local(m_cs);
                                if (this->Progress && m_br)
                                        this->Progress(this, m_nPos);
                                        m_br = nullptr;
                                })) {
                        if (!m_ash.GetAttachedClientSocket()->Sendable()) {
                            m_br = nullptr;
                            std::string s = m_ash.GetAttachedClientSocket()->GetErrorMsg();
#if defined(__ANDROID__) || defined(ANDROID)
                            return Utilities::ToWide(s.c_str(), s.size());
#else
                            CScopeUQueue su;
                            Utilities::ToWide(s.c_str(), s.size(), *su);
                            return (const wchar_t*)su->GetBuffer();
#endif
                        }
                    }
                    }
                }
                return res;
            }

        private:
            //disable copy constructor and assignment operator
            CStreamHelper(const CStreamHelper &sh);
            CStreamHelper& operator=(const CStreamHelper &sh);

            UINT64 SendDataFromClientToServer(std::istream &br) {
                bool ok = true;
                CScopeUQueue su;
                CUQueue &q = *su;
                UINT64 send = 0;
                unsigned int res = m_ash.GetAttachedClientSocket()->GetBytesInSendingBuffer();
                if (res > CStreamSerializationHelper::STREAM_CHUNK_SIZE)
                    return send;
                res = CStreamSerializationHelper::Read(br, q);
                while (res > 0 && ok) {
                    ok = m_ash.SendRequest(CStreamSerializationHelper::idWriteDataFromClientToServer, q.GetBuffer(), q.GetSize(), [this](CAsyncResult & ar) {
                        CAutoLock al(m_cs);
                        if (this->m_br)
                                SendDataFromClientToServer(*(this->m_br));
                        });
                    if (ok) {
                        send += res;
                        m_nPos += res;
                    } else {
                        m_br = nullptr;
                        break;
                    }
                    if (Progress) {
                        Progress(this, m_nPos);
                    }
                    res = m_ash.GetAttachedClientSocket()->GetBytesInSendingBuffer();
                    if (res > CStreamSerializationHelper::STREAM_CHUNK_SIZE * 10)
                        break;
                    res = CStreamSerializationHelper::Read(br, q);
                    if (res == 0) {
                        if (!m_ash.SendRequest(CStreamSerializationHelper::idUploadCompleted, [this](CAsyncResult & ar) {
                                CAutoLock al(m_cs);
                                if (m_br && this->Progress)
                                        this->Progress(this, this->m_nPos);
                                        m_br = nullptr;
                                }))
                        m_br = nullptr;
                    }
                }
                return send;
            }

        private:
            CAsyncServiceHandler &m_ash;
            std::istream *m_br;
            std::ostream *m_bw;
            CUCriticalSection m_cs; //for protecting m_br
            UINT64 m_nDownloadFileSize;
            UINT64 m_nPos;
        };

        template<typename THandler, typename TCS = CClientSocket>
        class CSocketPool {
        public:
            typedef std::shared_ptr<THandler> PHandler;
            typedef std::shared_ptr<TCS> PClientSocket;
            typedef std::function<bool(CSocketPool*, TCS*) > DDoSslAuthentication;
            typedef std::function<void(CSocketPool*, tagSocketPoolEvent, THandler*) > DSocketPoolEvent;

        private:

            struct cs_hash : public std::hash<PClientSocket> {

                inline size_t operator()(const PClientSocket & key) const {
                    return (size_t) key->GetHandle();
                }
            };

            struct cs_equal : public std::binary_function<PClientSocket, PClientSocket, bool> {

                inline bool operator()(const PClientSocket &s1, const PClientSocket & s2) const {
                    return (s1->GetHandle() == s2->GetHandle());
                }
            };

        public:
#if defined(__ANDROID__) || defined(ANDROID)
            typedef boost::unordered_map<PClientSocket, PHandler, cs_hash, cs_equal> CMapSocketHandler;
#else
            typedef std::unordered_map<PClientSocket, PHandler, cs_hash, cs_equal> CMapSocketHandler;
#endif
        public:

            CSocketPool(bool autoConn = true,
                    unsigned int recvTimeout = DEFAULT_RECV_TIMEOUT,
                    unsigned int connTimeout = DEFAULT_CONN_TIMEOUT,
                    unsigned int nServiceId = 0)
            : m_nPoolId(0),
            m_autoConn(autoConn),
            m_recvTimeout(recvTimeout),
            m_connTimeout(connTimeout),
            m_nServiceId(nServiceId) {
                if (!ClientCoreLoader.IsLoaded())
                    throw CUExCode("Client core library not accessible!", MB_BAD_OPERATION);
                CAutoLock al(g_csSpPool);
                m_vPool.push_back(this);
            }

            virtual ~CSocketPool() {
                ShutdownPool();
                CAutoLock al(g_csSpPool);
                for (auto it = m_vPool.begin(), e = m_vPool.end(); it != e; ++it) {
                    if (this == *it) {
                        m_vPool.erase(it);
                        break;
                    }
                }
            }

        private:
            CSocketPool(const CSocketPool &sp);
            CSocketPool& operator=(const CSocketPool &sp);

        public:

            static unsigned int GetSocketPools() {
                return ClientCoreLoader.GetNumberOfSocketPools();
            }

            DDoSslAuthentication DoSslServerAuthentication;
            DSocketPoolEvent SocketPoolEvent;

            inline const CMapSocketHandler& GetSocketHandlerMap() const {
                return m_mapSocketHandler;
            }

            inline CUCriticalSection& GetCriticalSection() {
                return m_cs;
            }

            std::vector<PHandler> GetAsyncHandlers() {
                std::vector<PHandler> v;
                {
                    CAutoLock al(m_cs);
                    for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                        v.push_back(it->second);
                    }
                }
                return v;
            }

            std::vector<PClientSocket> GetSockets() {
                std::vector<PClientSocket> v;
                {
                    CAutoLock al(m_cs);
                    for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                        v.push_back(it->first);
                    }
                }
                return v;
            }

            /// <summary>
            /// Seek an async handler on the min number of requests queued in memory and its associated socket connection
            /// </summary>
            /// <returns>An async handler if found; and null or nothing if no connection is found</returns>

            virtual PHandler Seek() {
                PHandler h;
                CAutoLock al(m_cs);
                for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                    if (it->first->GetConnectionState() != csSwitched)
                        continue;
                    if (!h)
                        h = it->second;
                    else {
                        unsigned int count0 = h->GetAttachedClientSocket()->GetCountOfRequestsInQueue();
                        unsigned int count1 = it->first->GetCountOfRequestsInQueue();
                        if (count0 > count1)
                            h = it->second;
                        else if (count0 == count1) {
                            UINT64 sent0 = h->GetAttachedClientSocket()->GetBytesSent();
                            UINT64 sent1 = it->first->GetBytesSent();
                            if (sent0 >= sent1)
                                h = it->second;
                        }
                    }
                }
                return h;
            }

            unsigned int GetQueues() {
                unsigned int queues = 0;
                CAutoLock al(m_cs);
                for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                    queues += (it->first->GetClientQueue().IsAvailable() ? 1 : 0);
                }
                return queues;
            }

            virtual PHandler SeekByQueue() {
                PHandler h;
                CAutoLock al(m_cs);
                bool automerge = ClientCoreLoader.GetQueueAutoMergeByPool(m_nPoolId);
                for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                    IClientQueue &cq = it->first->GetClientQueue();
                    if (automerge && h && !it->first->IsConnected())
                        continue;
                    if (!cq.IsAvailable())
                        continue;
                    if (!h)
                        h = it->second;
                    else {
                        UINT64 count0 = h->GetAttachedClientSocket()->GetClientQueue().GetMessageCount();
                        UINT64 count1 = cq.GetMessageCount();
                        if (count0 > count1 || (it->first->IsConnected() && !h->GetAttachedClientSocket()->IsConnected()))
                            h = it->second;
                    }
                }
                return h;
            }

            virtual PHandler SeekByQueue(const std::string &queueName) {
                PHandler h;
                std::string rawName;
                std::string qn(queueName);
                if (0 == qn.size())
                    return h;
#ifdef WIN32_64
                std::transform(qn.begin(), qn.end(), qn.begin(), ::tolower);
#endif
                CAutoLock al(m_cs);
                for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                    IClientQueue &cq = it->first->GetClientQueue();
                    if (!cq.IsAvailable())
                        continue;
                    if (cq.IsSecure())
                        rawName = qn + "_" + ClientCoreLoader.GetUClientAppName() + "_1.mqc";
                    else
                        rawName = qn + "_" + ClientCoreLoader.GetUClientAppName() + "_0.mqc";

                    std::string queueFileName = cq.GetQueueFileName();
                    size_t len = queueFileName.size();
                    size_t lenRaw = rawName.size();
                    if (lenRaw > len)
                        continue;
                    size_t pos = queueFileName.rfind(rawName);

                    //queue file name with full path
                    if (pos == 0)
                        return it->second;

                    //queue raw name only
                    if ((pos + lenRaw) == len)
                        return it->second;
                }
                return h;
            }

            inline bool IsAvg() {
                CAutoLock al(m_cs);
                return ClientCoreLoader.IsAvg(m_nPoolId);
            }

            inline bool GetQueueAutoMerge() {
                CAutoLock al(m_cs);
                return ClientCoreLoader.GetQueueAutoMergeByPool(m_nPoolId);
            }

            inline void SetQueueAutoMerge(bool autoMerger) {
                CAutoLock al(m_cs);
                ClientCoreLoader.SetQueueAutoMergeByPool(m_nPoolId, autoMerger);
            }

            inline unsigned int GetSocketsCreated() {
                CAutoLock al(m_cs);
                return (unsigned int) m_mapSocketHandler.size();
            }

            inline unsigned int GetPoolId() {
                CAutoLock al(m_cs);
                return m_nPoolId;
            }

            bool IsStarted() {
                CAutoLock al(m_cs);
                unsigned int count = ClientCoreLoader.GetThreadCount(m_nPoolId);
                return (count > 0);
            }

            virtual bool StartSocketPool(CConnectionContext **ppCCs, unsigned int threads, unsigned int socketsPerThread, bool avg = true, tagThreadApartment ta = taNone) {
                assert(ppCCs != nullptr);
                assert(*ppCCs != nullptr);
                assert(socketsPerThread != 0);
                assert(threads != 0);
                if (!StartSocketPool(socketsPerThread, threads, avg, ta))
                    return false;
                assert(threads == GetThreadsCreated());
                assert(socketsPerThread == GetSocketsPerThread());
                return PostProcess(ppCCs);
            }

            bool StartSocketPool(const CConnectionContext &cc, unsigned int socketsPerThread, unsigned int threads = 0, bool avg = true, tagThreadApartment ta = taNone) {
                unsigned int n;
                assert(socketsPerThread > 0);
                if (!StartSocketPool(socketsPerThread, threads, avg, ta))
                    return false;
                if (threads == 0)
                    threads = GetThreadsCreated();
                assert(threads == GetThreadsCreated());
                assert(socketsPerThread == GetSocketsPerThread());
                typedef CConnectionContext* PCConnectionContext;
                PCConnectionContext *ppCCs = new PCConnectionContext[threads];
                for (n = 0; n < threads; ++n) {
                    ppCCs[n] = new CConnectionContext[socketsPerThread];
                    for (unsigned int j = 0; j < socketsPerThread; ++j) {
                        ppCCs[n][j] = cc;
                    }
                }
                bool ok = PostProcess(ppCCs);
                for (n = 0; n < threads; ++n) {
                    delete[](ppCCs[n]);
                }
                delete[]ppCCs;
                return ok;
            }

            PHandler Lock(unsigned int timeout = (~0)) {
                return Lock(timeout, nullptr);
            }

            PHandler Lock(unsigned int timeout, PClientSocket csSameThread) {
                return Lock(timeout, csSameThread.get());
            }

            virtual PHandler Lock(unsigned int timeout, USocket_Client_Handle hSameThread) {
                unsigned int poolId;
                {
                    CAutoLock al(m_cs);
                    poolId = m_nPoolId;
                }
                USocket_Client_Handle h = ClientCoreLoader.LockASocket(poolId, timeout, hSameThread);
                if (!h)
                    return PHandler();
                return MapToHandler(h);
            }

            void Unlock(const THandler *h) {
                if (!h)
                    return;
                const CClientSocket *cs = h->GetAttachedClientSocket();
                Unlock(cs);
            }

            void Unlock(const PHandler &handler) {
                if (!handler)
                    return;
                const CClientSocket *cs = handler->GetAttachedClientSocket();
                Unlock(cs);
            }

            void Unlock(const PClientSocket &cs) {
                Unlock(cs.get());
            }

            virtual void Unlock(const CClientSocket *cs) {
                if (cs) {
                    Unlock(cs->GetHandle());
                }
            }

            virtual void Unlock(USocket_Client_Handle h) {
                unsigned int poolId;
                {
                    CAutoLock al(m_cs);
                    poolId = m_nPoolId;
                }
                ClientCoreLoader.UnlockASocket(poolId, h);
            }

            virtual void ShutdownPool() {
                ClientCoreLoader.DestroySocketPool(m_nPoolId);
                CAutoLock al(m_cs);
                m_nPoolId = 0;
            }

            inline unsigned int GetThreadsCreated() {
                CAutoLock al(m_cs);
                return ClientCoreLoader.GetThreadCount(m_nPoolId);
            }

            inline unsigned int GetDisconnectedSockets() {
                CAutoLock al(m_cs);
                return ClientCoreLoader.GetDisconnectedSockets(m_nPoolId);
            }

            inline unsigned int GetSocketsPerThread() {
                CAutoLock al(m_cs);
                return ClientCoreLoader.GetSocketsPerThread(m_nPoolId);
            }

            inline unsigned int GetLockedSockets() {
                CAutoLock al(m_cs);
                return ClientCoreLoader.GetLockedSockets(m_nPoolId);
            }

            inline unsigned int GetIdleSockets() {
                CAutoLock al(m_cs);
                return ClientCoreLoader.GetIdleSockets(m_nPoolId);
            }

            inline unsigned int GetConnectedSockets() {
                CAutoLock al(m_cs);
                return ClientCoreLoader.GetConnectedSockets(m_nPoolId);
            }

            inline bool DisconnectAll() {
                unsigned int poolId;
                {
                    CAutoLock al(m_cs);
                    poolId = m_nPoolId;
                }
                return ClientCoreLoader.DisconnectAll(poolId);
            }

            PHandler FindClosedOne() {
                CAutoLock al(m_cs);
                for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                    if (!it->first->IsConnected() && it->second.unique())
                        return it->second;
                }
                return PHandler();
            }

        protected:

            virtual void OnSocketPoolEvent(tagSocketPoolEvent spe, const PHandler &handler) {

            }

        private:

            bool PostProcess(CConnectionContext **ppCCs) {
                bool ok;
                unsigned int pos = 0;
                bool first = true;
                CAutoLock al(m_cs);
                unsigned int poolId = m_nPoolId;
                size_t size = m_mapSocketHandler.size();
                unsigned int socketsPerThread = ClientCoreLoader.GetSocketsPerThread(m_nPoolId);
                for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                    USocket_Client_Handle h = it->first->GetHandle();
                    if (ClientCoreLoader.IsOpened(h)) {
                        first = false;
                        continue;
                    }

                    PClientSocket cs = it->first;
                    cs->m_cc = ppCCs[pos / socketsPerThread][pos % socketsPerThread];

                    ClientCoreLoader.SetUserID(h, cs->m_cc.UserId.c_str());
                    ClientCoreLoader.SetEncryptionMethod(h, cs->m_cc.EncrytionMethod);

                    //we build connections asynchronously for all the sockets except the very first one
                    if (first) {
                        m_cs.unlock();
                        ok = (ClientCoreLoader.Connect(h, cs->m_cc.Host.c_str(), cs->m_cc.Port, true, cs->m_cc.V6) && ClientCoreLoader.WaitAll(h, (~0)));
                        m_cs.lock();
                        if (poolId != m_nPoolId || size != m_mapSocketHandler.size()) {
                            //stop here under extremely cases that other threads have just done something 
                            //on this pool of sockets during connecting the first socket
                            break;
                        }
                    } else
                        ok = ClientCoreLoader.Connect(h, cs->m_cc.Host.c_str(), cs->m_cc.Port, false, cs->m_cc.V6);
                    if (ok)
                        first = false;
                    ++pos;
                }
                unsigned int count = ClientCoreLoader.GetConnectedSockets(poolId);
                return (count > 0);
            }

            bool StartSocketPool(unsigned int socketsPerThread, unsigned int threads = 0, bool avg = true, tagThreadApartment ta = taNone) {
                if (IsStarted())
                    return true;
                unsigned int poolId = ClientCoreLoader.CreateSocketPool(&CSocketPool::SPE, socketsPerThread, threads, avg, ta);
                CAutoLock al(m_cs);
                m_nPoolId = poolId;
                return true;
            }

            PHandler MapToHandler(USocket_Client_Handle h) {
                CAutoLock al(m_cs);
                for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                    if (it->first->GetHandle() == h)
                        return it->second;
                }
                return PHandler();
            }

            PClientSocket MapToSocket(USocket_Client_Handle h) {
                CAutoLock al(m_cs);
                for (auto it = m_mapSocketHandler.begin(), end = m_mapSocketHandler.end(); it != end; ++it) {
                    if (it->first->GetHandle() == h)
                        return it->first;
                }
                return PClientSocket();
            }

            static void Set(PClientSocket &cs, USocket_Client_Handle h) {
                cs->Set(h);
            }

            static PClientSocket CreateEmptySocket() {
                return PClientSocket(new TCS());
            }

            static CSocketPool *Seek(unsigned int poolId) {
                CAutoLock al(g_csSpPool);
                for (auto it = m_vPool.begin(), e = m_vPool.end(); it != e; ++it) {
                    if ((*it)->GetPoolId() == poolId) {
                        return *it;
                    }
                }
                return nullptr;
            }

            static void CALLBACK SPE(unsigned int poolId, SPA::ClientSide::tagSocketPoolEvent spe, USocket_Client_Handle h) {
                CSocketPool *sp = Seek(poolId);
                switch (spe) {
                    case speShutdown:
                        if (sp) {
                            CAutoLock al(sp->m_cs);
                            sp->m_mapSocketHandler.clear();
                            sp->m_nPoolId = 0;
                        }
                        break;
                    case speStarted:
                    {
                        CAutoLock al(g_csSpPool);
                        for (auto it = m_vPool.rbegin(), end = m_vPool.rend(); it != end; ++it) {
                            if ((*it)->m_nPoolId == 0) {
                                sp = *it;
                                break;
                            }
                        }
                        sp->m_nPoolId = poolId;
                    }
                        break;
                    case speUSocketCreated:
                        if (sp) {
                            PClientSocket clientSocket = CSocketPool<THandler, TCS>::CreateEmptySocket();
                            Set(clientSocket, h);
                            ClientCoreLoader.SetRecvTimeout(h, sp->m_recvTimeout);
                            ClientCoreLoader.SetConnTimeout(h, sp->m_connTimeout);
                            ClientCoreLoader.SetAutoConn(h, sp->m_autoConn);
                            PHandler handler(new THandler(clientSocket.get()));
                            if (handler->GetSvsID() == 0)
                                handler->m_nServiceId = sp->m_nServiceId;
                            CAutoLock al(sp->m_cs);
                            sp->m_mapSocketHandler[clientSocket] = handler;
                        }
                        break;
                    case speUSocketKilled:
                        if (sp) {
                            CAutoLock al(sp->m_cs);
                            for (auto it = sp->m_mapSocketHandler.begin(), end = sp->m_mapSocketHandler.end(); it != end; ++it) {
                                if (it->first->GetHandle() == h) {
                                    sp->m_mapSocketHandler.erase(it);
                                    break;
                                }
                            }
                        }
                        break;
                    case speConnected:
                        if (sp && ClientCoreLoader.IsOpened(h)) {
                            ClientCoreLoader.SetSockOpt(h, soRcvBuf, 116800, slSocket);
                            ClientCoreLoader.SetSockOpt(h, soSndBuf, 116800, slSocket);
                            if (sp->DoSslServerAuthentication) {
                                if (ClientCoreLoader.GetEncryptionMethod(h) == TLSv1 && !sp->DoSslServerAuthentication(sp, sp->MapToSocket(h).get()))
                                    return;
                            }
                            ClientCoreLoader.SetPassword(h, sp->MapToSocket(h)->m_cc.Password.c_str());
                            bool ok = ClientCoreLoader.StartBatching(h);
                            ok = ClientCoreLoader.SwitchTo(h, sp->MapToHandler(h)->GetSvsID());
                            if (ok) {
                                ok = ClientCoreLoader.TurnOnZipAtSvr(h, sp->MapToSocket(h)->m_cc.Zip);
                                ok = ClientCoreLoader.SetSockOptAtSvr(h, soRcvBuf, 116800, slSocket);
                                ok = ClientCoreLoader.SetSockOptAtSvr(h, soSndBuf, 116800, slSocket);
                            }
                            ok = ClientCoreLoader.CommitBatching(h, false);
                        }
                        break;
                    case speQueueMergedFrom:
                        if (sp)
                            sp->m_pHFrom = sp->MapToHandler(h);
                        break;
                    case speQueueMergedTo:
                        if (sp) {
                            PHandler to = sp->MapToHandler(h);
                            sp->m_pHFrom->AppendTo(*to);
                            sp->m_pHFrom.reset();
                        }
                        break;
                    default:
                        break;
                }
                if (!sp)
                    return;
                const PHandler &handler = sp->MapToHandler(h);
                if (sp->SocketPoolEvent)
                    sp->SocketPoolEvent(sp, spe, handler.get());
                sp->OnSocketPoolEvent(spe, handler);
            }

        protected:
            CUCriticalSection m_cs;
            CMapSocketHandler m_mapSocketHandler; //locked by m_cs

        private:
            unsigned int m_nPoolId; //locked by m_cs
            bool m_autoConn;
            unsigned int m_recvTimeout;
            unsigned int m_connTimeout;
            unsigned int m_nServiceId;
            PHandler m_pHFrom;
            static std::vector<CSocketPool*> m_vPool; //protected by g_csSpPool
        };

        template<typename THandler, typename TCS>
        std::vector<CSocketPool<THandler, TCS>* > CSocketPool<THandler, TCS>::m_vPool;

        struct ReplicationSetting {
            static const unsigned int DEAFULT_TTL = 720 * 3600; //30 days

            ReplicationSetting()
            : QueueDir(CClientSocket::QueueConfigure::GetWorkDirectory()),
            NoAutoConn(false),
            TTL(DEAFULT_TTL),
            RecvTimeout(DEFAULT_RECV_TIMEOUT),
            ConnTimeout(DEFAULT_CONN_TIMEOUT) {

            }

            ReplicationSetting(const ReplicationSetting &rs)
            : QueueDir(rs.QueueDir),
            NoAutoConn(rs.NoAutoConn),
            TTL(rs.TTL),
            RecvTimeout(rs.RecvTimeout),
            ConnTimeout(rs.ConnTimeout) {

            }

            ReplicationSetting& operator=(const ReplicationSetting &rs) {
                if (this != &rs) {
                    QueueDir = rs.QueueDir;
                    NoAutoConn = rs.NoAutoConn;
                    TTL = rs.TTL;
                    RecvTimeout = rs.RecvTimeout;
                    ConnTimeout = rs.ConnTimeout;
                }
                return *this;
            }

            std::string QueueDir;
            bool NoAutoConn;
            unsigned int TTL;
            unsigned int RecvTimeout;
            unsigned int ConnTimeout;
        };

#if defined(__ANDROID__) || defined(ANDROID)
        typedef boost::unordered_map<std::string, CConnectionContext> CMapQNameConn;
#else
        typedef std::unordered_map<std::string, CConnectionContext> CMapQNameConn;
#endif

        template<typename THandler, typename TCS = CClientSocket>
        class CReplication {
            typedef CConnectionContext* PCConnectionContext;
            typedef std::shared_ptr<THandler> PHandler;
            typedef std::shared_ptr<TCS> PClientSocket;

        public:
#ifdef WIN32_64
            static const char DIR_SEP = '\\';
#else
            static const char DIR_SEP = '/';
#endif

            CReplication(const ReplicationSetting &rs)
            : m_pool(!rs.NoAutoConn, rs.RecvTimeout, rs.ConnTimeout),
            m_rs(rs) {
                assert(rs.QueueDir.size() > 0);
                assert(rs.QueueDir.find_first_not_of(" \n\r\t") == 0);
                assert(rs.QueueDir.rfind(DIR_SEP) == (rs.QueueDir.size() - 1));
                if (m_rs.TTL == 0)
                    m_rs.TTL = ReplicationSetting::DEAFULT_TTL;
            }

            virtual ~CReplication() {
                m_SourceHandler.reset();
                m_vTargetHandlers.clear();
            }

        public:

            inline unsigned int GetConnections() const {
                return m_pool.GetConnectedSockets();
            }

            inline bool IsReplicable() const {
                return m_mapQNameConn.size() > 1;
            }

            inline PHandler GetSourceHandler() const {
                return m_SourceHandler;
            }

            inline IClientQueue* GetSourceQueue() const {
                if (m_SourceHandler) {
                    return &(m_SourceHandler->GetAttachedClientSocket()->GetClientQueue());
                }
                return nullptr;
            }

            const std::vector<PHandler>& GetTargetHandlers() const {
                return m_vTargetHandlers;
            }

            const std::vector<IClientQueue*>& GetTargetQueues() const {
                return m_vTargetQueues;
            }

            unsigned int GetQueues() {
                return m_pool.GetQueues();
            }

            bool DoReplication() const {
                if (m_mapQNameConn.size() <= 1)
                    return false;
                std::vector<USocket_Client_Handle> vHandles;
                for (auto it = m_vTargetQueues.cbegin(), end = m_vTargetQueues.cend(); it != end; ++it) {
                    vHandles.push_back((*it)->GetHandle());
                }
                IClientQueue *src = GetSourceQueue();
                if (!src || !src->IsAvailable())
                    return false;
                return src->AppendTo(vHandles.data(), (unsigned int) vHandles.size());
            }

            inline size_t GetHosts() const {
                return m_mapQNameConn.size();
            }

            inline const ReplicationSetting& GetReplicationSetting() const {
                return m_rs;
            }

            virtual bool Send(unsigned short reqId, const unsigned char *buffer, unsigned int len) const {
                ResultHandler rh;
                PHandler src = GetSourceHandler();
                if (!src)
                    return false;
                IClientQueue *cq = &src->GetAttachedClientSocket()->GetClientQueue();
                if (!cq->IsAvailable())
                    return false;
                bool ok = src->SendRequest(reqId, buffer, len, rh);
                if (IsReplicable() && cq->GetJobSize() == 0)
                    ok = DoReplication();
                return ok;
            }

            bool Send(unsigned short reqId) const {
                return Send(reqId, (const unsigned char *) nullptr, 0);
            }

            bool Send(unsigned short reqId, const CUQueue &q) const {
                return Send(reqId, q.GetBuffer(), q.GetSize());
            }

            bool Send(unsigned short reqId, const CScopeUQueue &q) const {
                return Send(reqId, q->GetBuffer(), q->GetSize());
            }

            bool EndJob() const {
                IClientQueue *src = GetSourceQueue();
                if (!src || !src->IsAvailable())
                    return false;
                bool ok = src->EndJob();
                if (ok && IsReplicable())
                    ok = DoReplication();
                return ok;
            }

            bool StartJob() const {
                IClientQueue *src = GetSourceQueue();
                if (!src || !src->IsAvailable())
                    return false;
                return src->StartJob();
            }

            bool AbortJob() const {
                IClientQueue *src = GetSourceQueue();
                if (!src || !src->IsAvailable())
                    return false;
                return src->AbortJob();
            }

            template<typename T0>
            bool Send(unsigned short reqId, const T0 &t0) const {
                CScopeUQueue sb;
                sb << t0;
                return Send(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0, typename T1>
            bool Send(unsigned short reqId, const T0 &t0, const T1 &t1) const {
                CScopeUQueue sb;
                sb << t0 << t1;
                return Send(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0, typename T1, typename T2>
            bool Send(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2) const {
                CScopeUQueue sb;
                sb << t0 << t1 << t2;
                return Send(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0, typename T1, typename T2, typename T3>
            bool Send(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3) const {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3;
                return Send(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4>
            bool Send(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4) const {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4;
                return Send(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
            bool Send(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5) const {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5;
                return Send(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
            bool Send(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6) const {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6;
                return Send(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
            bool Send(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7) const {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7;
                return Send(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
            bool Send(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8) const {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
                return Send(reqId, sb->GetBuffer(), sb->GetSize());
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
            bool Send(unsigned short reqId, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, const T9 &t9) const {
                CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
                return Send(reqId, sb->GetBuffer(), sb->GetSize());
            }

            virtual bool Start(const CMapQNameConn &mapConnQueue, const char *rootQueueName = nullptr, tagThreadApartment ta = taNone) {
                const wchar_t *rootUID = L"SOCKETPRO_UREPLICATION_ROOT_DUMMY_NAME";
                m_mapQNameConn.clear();
                m_pool.ShutdownPool();
                assert(mapConnQueue.size() > 0);
                for (auto it = mapConnQueue.cbegin(), end = mapConnQueue.cend(); it != end; ++it) {
                    assert(it->first.size() > 0);
                    assert(!DoesQueueExist(it->first));
                    assert(it->first.find_first_not_of(" \n\r\t/\\") == 0);
                    assert(it->first.find_last_not_of(" \n\r\t/\\") == (it->first.size() - 1));
                    m_mapQNameConn[it->first] = it->second;
                }
                size_t n = 0;
                bool secure = false;
                size_t all = m_mapQNameConn.size();
                if (all > 1)
                    ++all;
                std::vector<std::string> vDbFullName;
                PCConnectionContext ppCCs[1];
                ppCCs[0] = new CConnectionContext[all];
                for (auto it = m_mapQNameConn.cbegin(), end = m_mapQNameConn.cend(); it != end; ++it, ++n) {
                    ppCCs[0][n] = it->second;
                    if (!secure && it->second.EncrytionMethod == TLSv1)
                        secure = true;
                    vDbFullName.push_back(m_rs.QueueDir + it->first);
                }
                if (all > 1) {
                    CConnectionContext last;
                    last.Host = "127.0.0.1";
                    last.Port = (~0);
                    last.UserId = rootUID;
                    last.Password = L"";
                    last.EncrytionMethod = secure ? TLSv1 : NoEncryption;
                    ppCCs[0][n] = last;
                    if (rootQueueName == nullptr || ::strlen(rootQueueName) == 0)
                        vDbFullName.push_back(m_rs.QueueDir + "urproot");
                    else
                        vDbFullName.push_back(m_rs.QueueDir + rootQueueName);
                }
                EndProcess(vDbFullName, secure, ppCCs, ta, rootUID);
                delete[](ppCCs[0]);
                return (m_pool.GetConnectedSockets() > 0);
            }

        protected:

            virtual bool DoSslServerAuthentication(TCS *cs) {
                return true;
            }

        private:
            //disable copy constructor and assignment operator
            CReplication(const CReplication &r);
            CReplication& operator=(const CReplication &r);

            bool DoesQueueExist(const std::string &qName) {
                std::string str1Cpy(qName);
#ifdef WIN32_64
                std::transform(str1Cpy.begin(), str1Cpy.end(), str1Cpy.begin(), ::tolower);
#endif
                for (auto it = m_mapQNameConn.cbegin(), end = m_mapQNameConn.cend(); it != end; ++it) {
                    std::string str2Cpy(it->first);
#ifdef WIN32_64
                    std::transform(str2Cpy.begin(), str2Cpy.end(), str2Cpy.begin(), ::tolower);
#endif
                    if (str2Cpy == str1Cpy)
                        return true;
                }
                return false;
            }

            void EndProcess(const std::vector<std::string>& vDbFullName, bool secure, PCConnectionContext *ppCCs, tagThreadApartment ta, const wchar_t *rootUID) {
                if (secure) {
                    m_pool.DoSslServerAuthentication = [this](CSocketPool<THandler, TCS> *sender, TCS * cs)->bool {
                        if (cs->GetConnectionState() < csConnected)
                            return true;
                        return this->DoSslServerAuthentication(cs);
                    };
                }
                int n = 0;
                bool ok = m_pool.StartSocketPool(ppCCs, 1, (unsigned int) (vDbFullName.size()), true, ta);
                m_SourceHandler.reset();
                m_vTargetHandlers.clear();
                m_vTargetQueues.clear();
                auto map = m_pool.GetSocketHandlerMap();
                size_t size = map.size();
                for (auto it = map.cbegin(), end = map.cend(); it != end; ++it) {
                    const std::string &qName = vDbFullName[n];
                    ok = it->first->GetClientQueue().StartQueue(qName.c_str(), m_rs.TTL, secure);
                    ++n;
                    if (size == 1) {
                        m_SourceHandler = it->second;
                        m_vTargetHandlers.push_back(m_SourceHandler);
                        m_vTargetQueues.push_back(&(it->first->GetClientQueue()));
                    } else {
                        if (it->first->GetUID() != rootUID) {
                            m_vTargetHandlers.push_back(it->second);
                            m_vTargetQueues.push_back(&(it->first->GetClientQueue()));
                        } else
                            m_SourceHandler = it->second;
                    }
                }
                if (size > 1) {
                    std::vector<USocket_Client_Handle> vHandles;
                    for (auto it = m_vTargetQueues.cbegin(), end = m_vTargetQueues.cend(); it != end; ++it) {
                        vHandles.push_back((*it)->GetHandle());
                    }
                    ok = GetSourceQueue()->EnsureAppending(vHandles.data(), (unsigned int) vHandles.size());
                }
                for (auto it = m_vTargetHandlers.begin(), end = m_vTargetHandlers.end(); it != end; ++it) {
                    ok = (*it)->GetAttachedClientSocket()->DoEcho();
                }
            }

        private:
            CSocketPool<THandler, TCS> m_pool;
            ReplicationSetting m_rs;
            CMapQNameConn m_mapQNameConn;
            PHandler m_SourceHandler;
            std::vector<PHandler> m_vTargetHandlers;
            std::vector<IClientQueue*> m_vTargetQueues;
        };
    }; //ClientSide
}; //SPA

#endif


#pragma once

#include "sspeer.h"

class CSSServer : public CSocketProServer
{
public:
	CSSServer(int nParam);
	~CSSServer();

protected:
	virtual bool OnSettingServer(unsigned int listeningPort, unsigned int maxBacklog, bool v6);
	virtual bool OnIsPermitted(USocket_Server_Handle h, const wchar_t* userId, const wchar_t *password, unsigned int serviceId);

private:
	bool AddServices();
	void SetOnlineMessage();

private:
	CSocketProService<CSSPeer> m_SSPeer;

private:
	CSSServer(const CSSServer &s);
	CSSServer& operator=(const CSSServer &s);
};

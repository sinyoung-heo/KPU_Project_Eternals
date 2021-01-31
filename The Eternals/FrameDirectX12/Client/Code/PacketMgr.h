#pragma once
#include "Include.h"
#include "Base.h"

#define ERR_CHECK

class CPacketMgr : public Engine::CBase
{
	DECLARE_SINGLETON(CPacketMgr)

private:
	explicit CPacketMgr();
	virtual ~CPacketMgr() = default;

public:
	HRESULT Ready_Server(ID3D12Device* pGraphicDevice, ID3D12GraphicsCommandList* pCommandList);
	HRESULT Connect_Server();

public:
	void	recv_packet();
	void	ProcessData(char* net_buf, size_t io_byte);
	void	ProcessPacket(char* ptr);

	void	send_login();

private:
	void	send_packet(void* packet);

private:
	virtual void Free();

private:
	ID3D12Device* m_pGraphicDevice = nullptr;
	ID3D12GraphicsCommandList* m_pCommandList = nullptr;
};

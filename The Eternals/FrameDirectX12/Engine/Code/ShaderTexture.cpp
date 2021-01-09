#include "ShaderTexture.h"

#include "GraphicDevice.h"
#include "Renderer.h"

USING(Engine)

CShaderTexture::CShaderTexture(ID3D12Device * pGraphicDevice, ID3D12GraphicsCommandList * pCommandList)
	: CShader(pGraphicDevice, pCommandList)
{
}

CShaderTexture::CShaderTexture(const CShaderTexture & rhs)
	: CShader(rhs)
{
	/*__________________________________________________________________________________________________________
	- Texture Buffer�� ���, ��ü���� ����ϴ� �ؽ�ó �̹����� ������ �ٸ��� ������,
	GameObject::LateInit_GameObject()���� Set_Shader_Texture() �Լ� ȣ���� ����
	Create_DescriptorHeaps(pVecTexture), Create_ConstantBuffer()�� ����.
	____________________________________________________________________________________________________________*/
}

HRESULT CShaderTexture::SetUp_ShaderConstantBuffer(const _uint& iNumTexture)
{
	m_pCB_MatrixDesc = CUploadBuffer<CB_MATRIX_DESC>::Create(m_pGraphicDevice, iNumTexture);
	NULL_CHECK_RETURN(m_pCB_MatrixDesc, E_FAIL);

	m_pCB_TexSpriteDesc = CUploadBuffer<CB_TEXSPRITE_DESC>::Create(m_pGraphicDevice, iNumTexture);
	NULL_CHECK_RETURN(m_pCB_TexSpriteDesc, E_FAIL);

	return S_OK;
}

HRESULT CShaderTexture::Ready_Shader()
{
	CShader::Ready_Shader();
	FAILED_CHECK_RETURN(Create_RootSignature(), E_FAIL);
	FAILED_CHECK_RETURN(Create_PipelineState(), E_FAIL);

	return S_OK;
}

void CShaderTexture::Begin_Shader(ID3D12DescriptorHeap* pTexDescriptorHeap, const _uint& iIdx)
{
	CRenderer::Get_Instance()->Set_CurPipelineState(m_pPipelineState);
	m_pCommandList->SetGraphicsRootSignature(m_pRootSignature);

	/*__________________________________________________________________________________________________________
	[ SRV�� ��Ʈ �����ڿ� ���´� ]
	____________________________________________________________________________________________________________*/
	ID3D12DescriptorHeap* pDescriptorHeaps[] = { pTexDescriptorHeap };
	m_pCommandList->SetDescriptorHeaps(_countof(pDescriptorHeaps), pDescriptorHeaps);


	CD3DX12_GPU_DESCRIPTOR_HANDLE SRV_DescriptorHandle(pTexDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	SRV_DescriptorHandle.Offset(iIdx, m_uiCBV_SRV_UAV_DescriptorSize);

	m_pCommandList->SetGraphicsRootDescriptorTable(0,		// RootParameter Index
												   SRV_DescriptorHandle);

	/*__________________________________________________________________________________________________________
	[ CBV�� ��Ʈ �����ڿ� ���´� ]
	____________________________________________________________________________________________________________*/
	m_pCommandList->SetGraphicsRootConstantBufferView(1,	// RootParameter Index
													  m_pCB_MatrixDesc->Resource()->GetGPUVirtualAddress() +
													  m_pCB_MatrixDesc->GetElementByteSize() * iIdx);

	m_pCommandList->SetGraphicsRootConstantBufferView(2,	// RootParameter Index
													  m_pCB_TexSpriteDesc->Resource()->GetGPUVirtualAddress() +
													  m_pCB_TexSpriteDesc->GetElementByteSize() * iIdx);
}


HRESULT CShaderTexture::Create_RootSignature()
{
	/*__________________________________________________________________________________________________________
	[ SRV�� ��� ������ ���̺��� ���� ]
	____________________________________________________________________________________________________________*/
	CD3DX12_DESCRIPTOR_RANGE SRV_Table;
	SRV_Table.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,	// �������� ���� - Shader Resource View.
				   1,								// �������� ���� - Texture2D�� ����.
				   0,								// ���̴� �μ����� ���� �������� ��ȣ. (register t0)
				   0);								// �������� ����.

	/*__________________________________________________________________________________________________________
	- ��Ʈ �Ű������� ���̺��̰ų�, ��Ʈ ������ �Ǵ� ��Ʈ ����̴�.
	____________________________________________________________________________________________________________*/
	CD3DX12_ROOT_PARAMETER RootParameter[3];
	RootParameter[0].InitAsDescriptorTable(1, &SRV_Table, D3D12_SHADER_VISIBILITY_PIXEL);

	RootParameter[1].InitAsConstantBufferView(0);	// register b0.
	RootParameter[2].InitAsConstantBufferView(1);	// register b1.

	auto StaticSamplers = Get_StaticSamplers();
	CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDesc(3,							// ��Ʈ �Ķ���� ����.(CBV 1, SRV 1 : �� 2��)
												  RootParameter,
												  (_uint)StaticSamplers.size(),	// ���÷� ����.
												  StaticSamplers.data(),		// ���÷� ������.
												  D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	
	/*__________________________________________________________________________________________________________
	[ RootSignature�� ���� ]
	____________________________________________________________________________________________________________*/
	ID3DBlob* pSignatureBlob	= nullptr;
	ID3DBlob* pErrorBlob		= nullptr;

	FAILED_CHECK_RETURN(D3D12SerializeRootSignature(&RootSignatureDesc,
													D3D_ROOT_SIGNATURE_VERSION_1,
													&pSignatureBlob,
													&pErrorBlob), E_FAIL);

	if (nullptr != pErrorBlob)
	{
		OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		return E_FAIL;
	}

	FAILED_CHECK_RETURN(m_pGraphicDevice->CreateRootSignature(0,
															  pSignatureBlob->GetBufferPointer(),
															  pSignatureBlob->GetBufferSize(),
															  IID_PPV_ARGS(&m_pRootSignature)),
															  E_FAIL);
	Engine::Safe_Release(pSignatureBlob);
	Engine::Safe_Release(pErrorBlob);

	return S_OK;
}

HRESULT CShaderTexture::Create_PipelineState()
{
	/*__________________________________________________________________________________________________________
	[ PipelineState �⺻ ���� ]
	____________________________________________________________________________________________________________*/
	ID3D12PipelineState*				pPipelineState = nullptr;
	vector<D3D12_INPUT_ELEMENT_DESC>	vecInputLayout;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC	PipelineStateDesc;
	ZeroMemory(&PipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	PipelineStateDesc.pRootSignature		= m_pRootSignature;
	PipelineStateDesc.SampleMask			= UINT_MAX;
	PipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	PipelineStateDesc.SampleDesc.Count		= CGraphicDevice::Get_Instance()->Get_MSAA4X_Enable() ? 4 : 1;
	PipelineStateDesc.SampleDesc.Quality	= CGraphicDevice::Get_Instance()->Get_MSAA4X_Enable() ? (CGraphicDevice::Get_Instance()->Get_MSAA4X_QualityLevels() - 1) : 0;
	PipelineStateDesc.DSVFormat				= DXGI_FORMAT_D24_UNORM_S8_UINT;


	// 0�� PipelineState Pass
	PipelineStateDesc.NumRenderTargets	= 2;
	PipelineStateDesc.RTVFormats[0]		= DXGI_FORMAT_R8G8B8A8_UNORM;
	PipelineStateDesc.RTVFormats[1]		= DXGI_FORMAT_R8G8B8A8_UNORM;
	vecInputLayout						= Create_InputLayout("VS_NORMAL_MAIN", "PS_NORMAL_MAIN");
	PipelineStateDesc.InputLayout		= { vecInputLayout.data(), (_uint)vecInputLayout.size() };
	PipelineStateDesc.VS				= { reinterpret_cast<BYTE*>(m_pVS_ByteCode->GetBufferPointer()), m_pVS_ByteCode->GetBufferSize() };
	PipelineStateDesc.PS				= { reinterpret_cast<BYTE*>(m_pPS_ByteCode->GetBufferPointer()), m_pPS_ByteCode->GetBufferSize() };
	PipelineStateDesc.BlendState		= Create_BlendState();
	PipelineStateDesc.RasterizerState	= CShader::Create_RasterizerState();
	PipelineStateDesc.DepthStencilState	= CShader::Create_DepthStencilState();

	FAILED_CHECK_RETURN(m_pGraphicDevice->CreateGraphicsPipelineState(&PipelineStateDesc,
																	  IID_PPV_ARGS(&pPipelineState)),
																	  E_FAIL);
	m_vecPipelineState.emplace_back(pPipelineState);
	CRenderer::Get_Instance()->Add_PipelineStateCnt();


	// 1�� PipelineState Pass
	PipelineStateDesc.RasterizerState = Create_RasterizerState(D3D12_FILL_MODE_WIREFRAME);

	FAILED_CHECK_RETURN(m_pGraphicDevice->CreateGraphicsPipelineState(&PipelineStateDesc, 
																	  IID_PPV_ARGS(&pPipelineState)), 
																	  E_FAIL);
	m_vecPipelineState.emplace_back(pPipelineState);
	CRenderer::Get_Instance()->Add_PipelineStateCnt();


	// 2�� PipelineState Pass
	vecInputLayout						= Create_InputLayout("VS_TEXTURE_SPRITE", "PS_TEXTURE_SPRITE");
	PipelineStateDesc.InputLayout		= { vecInputLayout.data(), (_uint)vecInputLayout.size() };
	PipelineStateDesc.VS				= { reinterpret_cast<BYTE*>(m_pVS_ByteCode->GetBufferPointer()), m_pVS_ByteCode->GetBufferSize() };
	PipelineStateDesc.PS				= { reinterpret_cast<BYTE*>(m_pPS_ByteCode->GetBufferPointer()), m_pPS_ByteCode->GetBufferSize() };
	PipelineStateDesc.BlendState		= Create_BlendState(true,
															D3D12_BLEND_ONE,
															D3D12_BLEND_ONE,
															D3D12_BLEND_OP_ADD,
															D3D12_BLEND_ONE,
															D3D12_BLEND_ONE,
															D3D12_BLEND_OP_ADD);
	PipelineStateDesc.RasterizerState	= CShader::Create_RasterizerState(D3D12_FILL_MODE_SOLID, D3D12_CULL_MODE_NONE);
	PipelineStateDesc.DepthStencilState = CShader::Create_DepthStencilState(true);

	FAILED_CHECK_RETURN(m_pGraphicDevice->CreateGraphicsPipelineState(&PipelineStateDesc, 
																	  IID_PPV_ARGS(&pPipelineState)), 
																	  E_FAIL);
	m_vecPipelineState.emplace_back(pPipelineState);
	CRenderer::Get_Instance()->Add_PipelineStateCnt();


	// 3�� PipelineState Pass - RenderTarget Texture
	m_pVS_ByteCode = Compile_Shader(L"../../Bin/Shader/ShaderTexture.hlsl", nullptr, "VS_MAIN", "vs_5_1");
	m_pPS_ByteCode = Compile_Shader(L"../../Bin/Shader/ShaderTexture.hlsl", nullptr, "PS_MAIN", "ps_5_1");

	_uint uiOffset = 0;
	vecInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, uiOffset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, uiOffset += sizeof(_vec3), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	PipelineStateDesc.InputLayout		= { vecInputLayout.data(), (_uint)vecInputLayout.size() };
	PipelineStateDesc.VS				= { reinterpret_cast<BYTE*>(m_pVS_ByteCode->GetBufferPointer()), m_pVS_ByteCode->GetBufferSize() };
	PipelineStateDesc.PS				= { reinterpret_cast<BYTE*>(m_pPS_ByteCode->GetBufferPointer()), m_pPS_ByteCode->GetBufferSize() };
	PipelineStateDesc.BlendState		= Create_BlendState();
	PipelineStateDesc.RasterizerState	= CShader::Create_RasterizerState();
	PipelineStateDesc.DepthStencilState	= CShader::Create_DepthStencilState();

	FAILED_CHECK_RETURN(m_pGraphicDevice->CreateGraphicsPipelineState(&PipelineStateDesc, 
																	  IID_PPV_ARGS(&pPipelineState)), 
																	  E_FAIL);
	m_vecPipelineState.emplace_back(pPipelineState);
	CRenderer::Get_Instance()->Add_PipelineStateCnt();


	return S_OK;
}

vector<D3D12_INPUT_ELEMENT_DESC> CShaderTexture::Create_InputLayout(string VS_EntryPoint,
																		   string PS_EntryPoint)
{
	vector<D3D12_INPUT_ELEMENT_DESC> vecInputLayout;

	m_pVS_ByteCode = Compile_Shader(L"../../Bin/Shader/ShaderTexture.hlsl", nullptr, VS_EntryPoint.c_str(), "vs_5_1");
	m_pPS_ByteCode = Compile_Shader(L"../../Bin/Shader/ShaderTexture.hlsl", nullptr, PS_EntryPoint.c_str(), "ps_5_1");

	_uint uiOffset = 0;
	vecInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, uiOffset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, uiOffset += sizeof(_vec3), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, uiOffset += sizeof(_vec3), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	return vecInputLayout;
}

D3D12_BLEND_DESC CShaderTexture::Create_BlendState(const _bool& bIsBlendEnable,
														  const D3D12_BLEND& SrcBlend,
														  const D3D12_BLEND& DstBlend,
														  const D3D12_BLEND_OP& BlendOp,
														  const D3D12_BLEND& SrcBlendAlpha,
														  const D3D12_BLEND& DstBlendAlpha,
														  const D3D12_BLEND_OP& BlendOpAlpha)
{
	D3D12_BLEND_DESC BlendDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

	// ������ ����.
	ZeroMemory(&BlendDesc, sizeof(D3D12_BLEND_DESC));
	BlendDesc.AlphaToCoverageEnable					= TRUE;
	BlendDesc.IndependentBlendEnable				= FALSE;
	BlendDesc.RenderTarget[0].BlendEnable			= bIsBlendEnable;
	BlendDesc.RenderTarget[0].LogicOpEnable			= FALSE;
	BlendDesc.RenderTarget[0].SrcBlend				= SrcBlend;
	BlendDesc.RenderTarget[0].DestBlend				= DstBlend;
	BlendDesc.RenderTarget[0].BlendOp				= BlendOp;
	BlendDesc.RenderTarget[0].SrcBlendAlpha			= SrcBlendAlpha;
	BlendDesc.RenderTarget[0].DestBlendAlpha		= DstBlendAlpha;
	BlendDesc.RenderTarget[0].BlendOpAlpha			= BlendOpAlpha;
	BlendDesc.RenderTarget[0].LogicOp				= D3D12_LOGIC_OP_NOOP;
	BlendDesc.RenderTarget[0].RenderTargetWriteMask	= D3D12_COLOR_WRITE_ENABLE_ALL;


	return BlendDesc;
}


CComponent * CShaderTexture::Clone()
{
	return new CShaderTexture(*this);
}

CShaderTexture * CShaderTexture::Create(ID3D12Device * pGraphicDevice, ID3D12GraphicsCommandList * pCommandList)
{
	CShaderTexture* pInstance = new CShaderTexture(pGraphicDevice, pCommandList);

	if (FAILED(pInstance->Ready_Shader()))
		Engine::Safe_Release(pInstance);

	return pInstance;
}

void CShaderTexture::Free()
{
	CShader::Free();

	Engine::Safe_Delete(m_pCB_MatrixDesc);
	Engine::Safe_Delete(m_pCB_TexSpriteDesc);
}
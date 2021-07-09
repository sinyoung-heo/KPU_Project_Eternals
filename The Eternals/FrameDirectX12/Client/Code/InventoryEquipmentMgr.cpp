#include "stdafx.h"
#include "InventoryEquipmentMgr.h"

IMPLEMENT_SINGLETON(CInventoryEquipmentMgr)

CInventoryEquipmentMgr::CInventoryEquipmentMgr()
{
	m_vecInventorySlot.reserve(80);
}

_uint uiInventorySlotIdx = 0;
void CInventoryEquipmentMgr::Add_InventorySlot(CInventoryItemSlot* pSlot)
{
	if (nullptr != pSlot)
	{
		m_vecInventorySlot.emplace_back(pSlot);
		pSlot->Set_ItemSlotIdx(uiInventorySlotIdx);

		++uiInventorySlotIdx;
	}
}

void CInventoryEquipmentMgr::Push_ItemInventory(const char& chItemType, const char& chItemName, const _int& iCnt)
{
	if (m_uiCurSlotSize >= m_uiMaxSlotSize)
	{
		cout << "Inventory is Full ! " << endl;
		return;
	}

	if (ItemType_Potion == chItemType)
	{
		// 현재 인벤토리에 포션이 있는지 탐색.
		for (auto& pSlot : m_vecInventorySlot)
		{
			if (chItemType == pSlot->Get_CurItemInfo().chItemType &&
				chItemName == pSlot->Get_CurItemInfo().chItemName)
			{
				pSlot->Add_PotionCnt(iCnt);
				return;
			}
		}

		// 현재 인벤토리에 포션이 없다면 빈칸에 포션을 새로 생성.
		for (auto& pSlot : m_vecInventorySlot)
		{
			if (NO_ITEM == pSlot->Get_CurItemInfo().chItemType)
			{
				pSlot->Set_CurItemInfo(chItemType, chItemName, iCnt);
				++m_uiCurSlotSize;

				if (Potion_HP == chItemName)
					m_pHpPotionSlot = pSlot;
				else
					m_pMpPotionSlot = pSlot;

				break;
			}
		}
	}
	else
	{
		for (auto& pSlot : m_vecInventorySlot)
		{
			if (NO_ITEM == pSlot->Get_CurItemInfo().chItemType)
			{
				pSlot->Set_CurItemInfo(chItemType, chItemName, iCnt);
				++m_uiCurSlotSize;

				break;
			}
		}
	}

}

void CInventoryEquipmentMgr::Pop_ItemInventory(const char& chItemType, const char& chItemName, const _int& iCnt)
{
	if (m_uiCurSlotSize <= 0)
	{
		cout << "Inventory is Empty ! " << endl;
		return;
	}

	if (ItemType_Potion == chItemType)
	{
		// 현재 인벤토리에 포션이 있는지 탐색.
		for (auto& pSlot : m_vecInventorySlot)
		{
			if (chItemType == pSlot->Get_CurItemInfo().chItemType &&
				chItemName == pSlot->Get_CurItemInfo().chItemName)
			{
				pSlot->Add_PotionCnt(iCnt);

				if (pSlot->Get_CurItemCnt() <= 0)
				{
					pSlot->Set_CurItemCnt(0);
					pSlot->Set_CurItemInfo(NO_ITEM, NO_ITEM);
					--m_uiCurSlotSize;

					//if (Potion_HP == chItemName)
					//	m_pHpPotionSlot = nullptr;
					//else
					//	m_pMpPotionSlot = nullptr;
				}

				break;
			}
		}
	}
	else
	{
		for (auto& pSlot : m_vecInventorySlot)
		{
			if (chItemType == pSlot->Get_CurItemInfo().chItemType &&
				chItemName == pSlot->Get_CurItemInfo().chItemName)
			{
				ITEM_DESC tItemInfo = pSlot->Get_CurItemInfo();

				pSlot->Set_CurItemInfo(NO_ITEM, NO_ITEM, iCnt);
				--m_uiCurSlotSize;

				break;
			}
		}
	}

}

void CInventoryEquipmentMgr::Pop_ItemInventory(const _uint& uiIdx)
{
	if (uiIdx < 0 || uiIdx >= m_uiMaxSlotSize)
	{
		cout << "Error Idx ! " << endl;
		return;
	}

	if (NO_ITEM != m_vecInventorySlot[uiIdx]->Get_CurItemInfo().chItemType)
	{
		ITEM_DESC tItemInfo = m_vecInventorySlot[uiIdx]->Get_CurItemInfo();
		
		m_vecInventorySlot[uiIdx]->Set_CurItemInfo(NO_ITEM, NO_ITEM, 0);
		--m_uiCurSlotSize;
	}
}

void CInventoryEquipmentMgr::Pop_ItemInventory()
{
	if (m_uiCurSlotSize <= 0)
	{
		cout << "Inventory is Empty ! " << endl;
		return;
	}

	for (auto& pSlot : m_vecInventorySlot)
	{
		if (NO_ITEM != pSlot->Get_CurItemInfo().chItemType)
		{
			ITEM_DESC tItemInfo = pSlot->Get_CurItemInfo();

			pSlot->Set_CurItemInfo(NO_ITEM, NO_ITEM, 0);
			--m_uiCurSlotSize;

			break;
		}
	}
}

void CInventoryEquipmentMgr::Add_EquipmentSlot(wstring wstrTag, CEquipmentItemSlot* pSlot)
{
	auto iter_find = m_mapEquipmentSlot.find(wstrTag);

	if (iter_find == m_mapEquipmentSlot.end())
		m_mapEquipmentSlot.emplace(wstrTag, pSlot);
}

void CInventoryEquipmentMgr::Free()
{
	m_pInventoryCanvas   = nullptr;
	m_pInventorySwapSlot = nullptr;
	m_pEquipmentCanvas   = nullptr;

	m_pHpPotionSlot = nullptr;
	m_pMpPotionSlot = nullptr;
}

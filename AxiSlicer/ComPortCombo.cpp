// ComPortCombo.cpp : Implementierungsdatei
//

#include "stdafx.h"
#include "ComPortCombo.h"

// CComPortCombo

IMPLEMENT_DYNAMIC(CComPortCombo, CComboBox)
CComPortCombo::CComPortCombo()
{
	m_bNoneItem = TRUE;								// First item is "<None>"
	m_bOnlyPhysical = FALSE;						// Include virtual COM ports
	m_bOnlyPresent = TRUE;							// Only present ports
	m_strNone = _T("<None>");
}

CComPortCombo::~CComPortCombo()
{
}


BEGIN_MESSAGE_MAP(CComPortCombo, CComboBox)
END_MESSAGE_MAP()



// CComPortCombo-Meldungshandler

// Get item data for current selection (port number with additional flags)
int CComPortCombo::GetCurData() const
{
	int nSel = GetCurSel();
	return nSel >= 0 ? static_cast<int>(GetItemData(nSel)) : 0;
}

// Fill the combo box.
BOOL CComPortCombo::InitList(int nDefPort /*= -1*/)
{
	// Should be drop-down list without edit field.
	// CBS_DROPDOWNLIST = 3 while CBS_SIMPLE = 1 and CBS_DROPDOWN = 2
	ASSERT((GetStyle() & CBS_DROPDOWNLIST) == CBS_DROPDOWNLIST);

	m_nDefPort = nDefPort;							// use member var for access by call back function
	if (m_nDefPort < 0)								// when no default port specified
		m_nDefPort = GetPortNum();					//  use the currently selected one upon re-loading
	ResetContent();
	if (m_bNoneItem)
	{
		int nItem = AddString(m_strNone.GetString());
		SetItemData(nItem, 0);
	}

	CEnumDevices Enum;
	BOOL bResult = Enum.EnumSerialPorts(this, CallbackWrapper, m_bOnlyPresent);

	// If no port pre-selected and none item used or only one port present, select first item.
	if (m_nDefPort < GetCount())
		SetCurSel(m_nDefPort);
	else
		SetCurSel(0);

	return bResult;
}

// Static wrapper function to add list items.
// Called by the CEnumDevices enumeration function.
/*static*/ void CComPortCombo::CallbackWrapper(CObject* pObject, const CEnumDevInfo* pInfo)
{
	ASSERT(pObject != NULL);
	CComPortCombo* pThis = reinterpret_cast<CComPortCombo*>(pObject);
	ASSERT(pThis->IsKindOf(RUNTIME_CLASS(CComPortCombo)));
	pThis->AddItem(pInfo);
}

void CComPortCombo::AddItem(const CEnumDevInfo* pInfo)
{
	ASSERT(pInfo != NULL);

	if (pInfo->m_nPortNum > 0 &&
		(!m_bOnlyPhysical || !(pInfo->m_nPortNum & DATA_VIRTUAL_MASK)))
	{
		int nItem = AddString(pInfo->m_strName.GetString());
		SetItemData(nItem, static_cast<DWORD>(pInfo->m_nPortNum));
		if ((pInfo->m_nPortNum & DATA_PORT_MASK) == m_nDefPort)
			SetCurSel(nItem);
	}
}

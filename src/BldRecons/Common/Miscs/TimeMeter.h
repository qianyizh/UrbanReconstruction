#pragma once

class CTimeMeter
{
public:
	CTimeMeter(void);
	~CTimeMeter(void);

public:
	DWORD m_dwStart;
	DWORD m_dwEnd;

	void Start();
	void End();
	void Print();
};

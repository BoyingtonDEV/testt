﻿// AtumSound.cpp: implementation of the CAtumSound class.
//
//////////////////////////////////////////////////////////////////////
#include "StdAfx.h"

// explicitly include the correct header (panoskj)
#include "GameDataLast.h"

#include "AtumSound.h"

#include "MusicMP3Ex.h"

#include "AtumApplication.h"
#include "ShuttleChild.h"
#include "CharacterChild.h"				// 2005-07-21 by ispark
#include "INFGameMain.h"
#include "Background.h"
#include "dxutil.h"
#include "dsutil.h"
#include "INFMp3Player.h"

// 2007-07-24 by bhsohn 나레이션 mp3추가
#define	NARRATION_VOLUME		50

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAtumSound::CAtumSound()
{
	m_bPlayMusic = FALSE;
	m_pMusic = NULL;
	// 2007-07-24 by bhsohn 나레이션 mp3추가
	m_pNarrationMusic = NULL;

	m_fSetSoundGroundCheckTime = 0.0f;
	util::zero(m_strBackMusic);

	m_pSoundManager		= NULL;
	m_p3DSoundManager	= NULL;
	m_pDSListener		= NULL;

	util::zero(m_dsListenerParams);

	m_pGameData = NULL;

	m_nMusicVolume = OPTION_DEFAULT_MUSICVOLUME;
	m_nOnlyBackMusicState = -1;
	m_bOnlyBackMusic = FALSE;

	m_bNotDevice = TRUE;								// 2006-09-15 by ispark, 기본적으로 활성

	// 2007-07-24 by bhsohn 나레이션 mp3추가
	m_bPlayNarrationMusic = FALSE;
	util::zero(m_strNarrationMusic);			// 나레이션 뮤직 이름

	//m_nSoundType = -1;	
}


HRESULT CAtumSound::SetResourceFile(char* szFileName)
{
	FLOG("CINFGameMain::SetResourceFile(char* szFileName)");

	util::del(m_pGameData);

	m_pGameData = new CGameData;

	m_pGameData->SetFile(szFileName, false, nullptr, 0, false);

	return S_OK;
}



HRESULT	CAtumSound::InitDeviceObjects()
{
	SetResourceFile(".\\Res-Snd\\sound.dat");

	if (!m_p3DSoundManager)
	{
		// Create a static IDirectSound in the CSound class.  
		// Set coop level to DSSCL_PRIORITY, and set primary buffer 
		// format to stereo, 22kHz and 16-bit output.
		m_p3DSoundManager = new CSoundManager;
		HRESULT hr = m_p3DSoundManager->Initialize(g_pD3dApp->GetHwnd(), DSSCL_PRIORITY, 2, 2250, 16);
		if(SUCCEEDED(hr))
		{
			hr = m_p3DSoundManager->SetPrimaryBufferFormat(2,2250,16);
		}

		if(FAILED(hr))
		{
			MessageBox(g_pD3dApp->GetHwnd(), STRERR_C_SOUND_0001, 
				STRMSG_WINDOW_TEXT, MB_OK | MB_ICONERROR );

			m_bNotDevice = FALSE;
			return S_OK;
		}
		// Get the 3D listener, so we can control its params
		hr |= m_p3DSoundManager->Get3DListenerInterface( &m_pDSListener);

		if(FAILED(hr))
		{
			MessageBox(g_pD3dApp->GetHwnd(), STRERR_C_SOUND_0002, 
				STRMSG_WINDOW_TEXT, MB_OK | MB_ICONERROR );

			m_bNotDevice = FALSE;
			return S_OK;
		}

		m_dsListenerParams.dwSize = sizeof(DS3DLISTENER);
		m_pDSListener->GetAllParameters(&m_dsListenerParams);
	}

	if (!m_pSoundManager)
	{
		m_pSoundManager = new CSoundManager;
		HRESULT hr = m_pSoundManager->Initialize(g_pD3dApp->GetHwnd(), DSSCL_PRIORITY, 2, 2250, 16);		
		if(SUCCEEDED(hr))
		{
			hr = m_pSoundManager->SetPrimaryBufferFormat(2,2250,16);
		}
	}

	// 2009-01-20 by bhsohn 사운드 시스템 변경
// 	m_pMusic = new CMusicMP3();
// 	// 2007-07-24 by bhsohn 나레이션 mp3추가
// 	m_pNarrationMusic = new CMusicMP3();
	m_pMusic = new CMusicMP3Ex();	
	m_pNarrationMusic = new CMusicMP3Ex();
	// end 2009-01-20 by bhsohn 사운드 시스템 변경

	return S_OK;
}

void CAtumSound::Tick()
{
// 2005-01-03 by jschoi  주석 - 사용안함
//	for(int i=0; i < m_vectorSoundPtr.size(); i++)
//	{
//		m_vectorSoundPtr[i]->SetAllParameter();
//	}
	// 현재 쓰이지 않음
//	if(m_fSetSoundGroundCheckTime > 0.0f)
//	{
//		m_fSetSoundGroundCheckTime -= g_pD3dApp->GetElapsedTime();
//	}
//	if(m_fSetSoundGroundCheckTime <= 0.0f)
//	{
//		m_fSetSoundGroundCheckTime = 5.0f;
//		CheckD3DSoundGround();
//	}
}
HRESULT	CAtumSound::DeleteDeviceObjects()
{
	SAFE_RELEASE(m_pDSListener);
	vectorSoundPtr::iterator itSound = m_vectorSoundPtr.begin();
	for(; itSound != m_vectorSoundPtr.end(); itSound++)
	{
		util::del(*itSound);
	}
	m_vectorSoundPtr.clear();		
	util::del(m_pSoundManager);
	util::del(m_p3DSoundManager);

	if(m_pMusic)
	{
		m_pMusic->Atum_MusicStop();
		util::del(m_pMusic);
		m_bPlayMusic = FALSE;
	}
	// 2007-07-24 by bhsohn 나레이션 mp3추가
	if(m_pNarrationMusic)
	{
		m_pNarrationMusic->Atum_MusicStop();
		util::del(m_pNarrationMusic);		
	}
	return S_OK;
}

// helper; does what its name says
D3DXVECTOR3 CalcRelativeSoundPos(const D3DXVECTOR3& vecPos, const D3DXMATRIX& matShuttle)
{
	D3DXMATRIX matInv;
	D3DXVECTOR3 vecSoundPos;

	D3DXMatrixInverse(&matInv, nullptr, &matShuttle);
	D3DXVec3TransformCoord(&vecSoundPos, &vecPos, &matInv);

	vecSoundPos.x = -vecSoundPos.x;
	vecSoundPos.z = -vecSoundPos.z;

	return vecSoundPos;
}


// 성우 스크립트에서만 사용. 다른데 사용하려면 문의할것. 2004-12-15, by dhkwon
// creates a sound, adds it in the sounds vector and plays it
void CAtumSound::PlayD3DSound(int nType, char* pBuffer, D3DXVECTOR3 vPos, BOOL i_b3DSound/*=TRUE*/)
{
	// 2006-09-15 by ispark, 장치 활성 유무
	// 2010. 07. 07 by jskim 모선전시 사운드 버그 수정

	if (!m_bNotDevice || g_pSOption->sSoundVolume == -10000)

		return;

	CSound* pTmSound = nullptr;

	WAVEFORMATEX wfx;

	util::zero(wfx);

	DWORD dwWaveFormLen;

	auto pWaveRaw = SetWaveFormatEx(PBYTE(pBuffer), dwWaveFormLen, &wfx);

	if (!pWaveRaw) return;

	DWORD			dwCreationFlag = DSBCAPS_CTRLVOLUME;
	GUID			guid = GUID_NULL;
	CSoundManager	*pSoundManager = NULL;

	if (i_b3DSound && wfx.nChannels == 1)
	{
		dwCreationFlag |= DSBCAPS_CTRL3D;
		guid = DS3DALG_NO_VIRTUALIZATION;
		pSoundManager = m_p3DSoundManager;
	}
	else
	{
		pSoundManager = m_pSoundManager;
	}


	if (pSoundManager == nullptr ||
		FAILED(pSoundManager->CreateFromMemory(&pTmSound,
			pWaveRaw,
			dwWaveFormLen,
			&wfx,
			dwCreationFlag,
			guid,
			COUNT_3DSOUND_PLAY_MULTI_BUFFER,
			i_b3DSound)))
	{
		return;
	}

	pTmSound->m_nSoundType = nType;


	m_vectorSoundPtr.push_back(pTmSound);

	// Calculate relative sound position

	auto vecSoundPos = CalcRelativeSoundPos(vPos, g_pShuttleChild->m_mMatrix);

	if (i_b3DSound) pTmSound->Set3DSoundPosition(&vecSoundPos);

	if (FAILED(pTmSound->Play(0, 0, g_pSOption->sSoundVolume)))
		
		return;
}

// finds a sound and plays it
void CAtumSound::PlayD3DSound(int nType,D3DXVECTOR3 vPos, BOOL i_b3DSound/*=TRUE*/)
{
	if (!m_bNotDevice || g_pSOption->sSoundVolume == -10000)

		return;

	if (!i_b3DSound) util::zero(vPos);

	auto pTmSound = FindSCSoundWithSoundType(nType);

	if (!pTmSound)
	{
		char strWave[16] { };

		snprintf(strWave, 16, "%08d", nType);

		auto pHeader = m_pGameData->FindFromFile(strWave);

		if (!pHeader)
		{
			DBGOUT("Sound %s not found in %s\n", strWave, m_pGameData->GetZipFilePath());

			return;
		}

		WAVEFORMATEX wfx;

		util::zero(wfx);

		DWORD dwWaveFormLen;

		auto pWaveRaw = SetWaveFormatEx(PBYTE(pHeader->m_pData), dwWaveFormLen, &wfx);

		if (!pWaveRaw) return;

		DWORD			dwCreationFlag = DSBCAPS_CTRLVOLUME;
		GUID			guid = GUID_NULL;

		CSoundManager	*pSoundManager;

		if (i_b3DSound && wfx.nChannels == 1)
		{
			dwCreationFlag |= DSBCAPS_CTRL3D;
			guid = DS3DALG_NO_VIRTUALIZATION;
			pSoundManager = m_p3DSoundManager;
		}
		else pSoundManager = m_pSoundManager;

		if (!pSoundManager ||
			FAILED(pSoundManager->CreateFromMemory(&pTmSound, 
						  pWaveRaw,
						  dwWaveFormLen,
                          &wfx,
                          dwCreationFlag,
						  guid,
						  COUNT_3DSOUND_PLAY_MULTI_BUFFER,
						  i_b3DSound)))
		{
			util::del(pHeader);// 2005-02-04 by jschoi
			return;
		}
		pTmSound->m_nSoundType = nType;
		///////////////////////////////////////////////////////////////////////////////
		// Vector에 추가
		m_vectorSoundPtr.push_back(pTmSound);

		util::del(pHeader);// 2005-02-04 by jschoi
	}
	// 2010. 07. 07 by jskim 모선전시 사운드 버그 수정
	//pTmSound->Stop();
	//end 2010. 07. 07 by jskim 모선전시 사운드 버그 수정
	D3DXVECTOR3 tmVec3SoundPos = vPos;
	if(i_b3DSound)
	{
		if(D3DXVec3Length(&(vPos - g_pShuttleChild->m_vPos)) >= 1500.0f)
		{
			return;
		}

		D3DXMATRIX matPlayer = g_pShuttleChild->m_mMatrix;
		D3DXMATRIX matInv = g_pShuttleChild->m_mMatrix;

		D3DXMatrixInverse( &matInv, 0, &matPlayer);
		D3DXVec3TransformCoord( &tmVec3SoundPos, &vPos, &matInv);

		tmVec3SoundPos.x = -tmVec3SoundPos.x;
		tmVec3SoundPos.z = -tmVec3SoundPos.z;

		// 2010. 07. 07 by jskim 모선전시 사운드 버그 수정
		pTmSound->Set3DSoundPosition(&tmVec3SoundPos);
		//end 2010. 07. 07 by jskim 모선전시 사운드 버그 수정
	}	
	
	if(nType == SOUND_FLYING_M_A_GEAR 
		|| nType == SOUND_FLYING_B_I_GEAR 
		|| nType == SOUND_HIGH_BOOSTER 
		|| nType == SOUND_LOW_BOOSTER
		|| nType == SOUND_GROUND_MOVING_A_GEAR)
//		|| nType == SOUND_MOVING_ON_WATER)
	{
		if(FAILED(pTmSound->Play(0, DSBPLAY_LOOPING, g_pSOption->sSoundVolume)))
		{
			DBGOUT(STRERR_C_SOUND_0003, nType);
			return;
		}
	}
	else if(nType == SOUND_MISSILE_WARNNING)								// 2005-07-11 by ispark
	{
		if (pTmSound->IsSoundPlaying())								// Loop를 다 돌았는가를 판단
		{
			return;
		}

		if(FAILED(pTmSound->Play(0, DSBPLAY_LOOPING, g_pSOption->sSoundVolume)))
		{
			return;
		}
	}
	else
	{
		if(FAILED(pTmSound->Play(0, 0, g_pSOption->sSoundVolume)))
		{
			return;
		}
	}
}

// WAVEFORMATEX 를 V팅하고 실제 wave form data 길이를 리턴한다.
BYTE * CAtumSound::SetWaveFormatEx(BYTE* pData, DWORD& dwWaveFormLen, WAVEFORMATEX* pwfx )
{
	typedef struct
	{
		char id[4];
		DWORD len;
		char WaveID[4];
	} riff_hdr;
	
	typedef struct
	{
		char id[4];
		DWORD len;
	} chunk_hdr;
	
	riff_hdr	riffHdr;
	memcpy(&riffHdr, pData, sizeof(riff_hdr));
	pData += sizeof(riff_hdr);

	chunk_hdr	chHdr;
	memcpy(&chHdr, pData, sizeof(chunk_hdr));
	pData += sizeof(chunk_hdr);
	memcpy(pwfx, pData, chHdr.len);
	pData += chHdr.len;
	
	chunk_hdr	chDataHdr;	
	for(int i = 0; i < 5; i++)
	{
		memcpy(&chDataHdr, pData, sizeof(chunk_hdr));
		pData += sizeof(chunk_hdr);
		if(chDataHdr.id[0] == 'd' && chDataHdr.id[1] == 'a' 
			&& chDataHdr.id[2] == 't' && chDataHdr.id[3] == 'a')
		{
			dwWaveFormLen = chDataHdr.len;
			return pData;
		}
		pData += chDataHdr.len;
	}

	dwWaveFormLen = 0;	
	return NULL;
}

VOID CAtumSound::PlayD3DSound(char * str, D3DXVECTOR3 vPos, BOOL i_b3DSound/*=TRUE*/)
{
	if (!m_bNotDevice || g_pSOption->sSoundVolume == -10000)

		return;

	if (!i_b3DSound) util::zero(vPos);

	int nSType;
	char strWave[20];

	memset(strWave,0x00,sizeof(strWave));
	memcpy(strWave, str, 8);
	nSType = atoi(strWave);

	auto pTmSound = FindSCSoundWithSoundType(nSType);

	if (!pTmSound)
	{
		DataHeader* pHeader = NULL;
		pHeader = m_pGameData->FindFromFile(strWave);

		if(pHeader == NULL) // by dhkwon 2005-11-23, NULL체크
		{
			DBGOUT("Sound File Error(%s)\n", strWave);
			return;
		}

		WAVEFORMATEX wfx;
		memset(&wfx, 0x00, sizeof(WAVEFORMATEX));
		DWORD	dwWaveFormLen;

		BYTE * pWaveRaw = SetWaveFormatEx((BYTE*)pHeader->m_pData, dwWaveFormLen, &wfx );

		if(NULL == pWaveRaw)
		{
			util::del(pHeader);// 2005-02-04 by jschoi
			return;
		}

		DWORD			dwCreationFlag = DSBCAPS_CTRLVOLUME;
		GUID			guid = GUID_NULL;
		CSoundManager	*pSoundManager = NULL;
		if(i_b3DSound
			&& wfx.nChannels == 1)
		{
			dwCreationFlag |= DSBCAPS_CTRL3D;
			guid = DS3DALG_NO_VIRTUALIZATION;
			pSoundManager = m_p3DSoundManager;
		}
		else
		{
//			i_b3DSound = FALSE;
			pSoundManager = m_pSoundManager;
		}

		if(FAILED(pSoundManager->CreateFromMemory( (CSound**)&pTmSound, 
						  pWaveRaw,
						  dwWaveFormLen,
                          &wfx,
                          dwCreationFlag,
						  guid,
						  COUNT_3DSOUND_PLAY_MULTI_BUFFER,
						  i_b3DSound)))
		{
			util::del(pHeader);// 2005-02-04 by jschoi
			return;
		}
		pTmSound->m_nSoundType = nSType;
		///////////////////////////////////////////////////////////////////////////////
		// Vector에 추가
		m_vectorSoundPtr.push_back(pTmSound);
		util::del(pHeader);// 2005-02-04 by jschoi
	}
	// 2010. 07. 07 by jskim 모선전시 사운드 버그 수정
//	pTmSound->Stop();
	//end 2010. 07. 07 by jskim 모선전시 사운드 버그 수정
	D3DXVECTOR3 tmVec3SoundPos = vPos;
	if(i_b3DSound)
	{
		if(D3DXVec3Length(&(vPos - g_pShuttleChild->m_vPos)) >= 1500.0f)
		{
			return;
		}

		D3DXMATRIX matPlayer = g_pShuttleChild->m_mMatrix;
		D3DXMATRIX matInv = g_pShuttleChild->m_mMatrix;

		D3DXMatrixInverse( &matInv, 0, &matPlayer);
		D3DXVec3TransformCoord( &tmVec3SoundPos, &vPos, &matInv);

		tmVec3SoundPos.x = -tmVec3SoundPos.x;
		tmVec3SoundPos.z = -tmVec3SoundPos.z;
		// 2010. 07. 07 by jskim 모선전시 사운드 버그 수정
		pTmSound->Set3DSoundPosition(&tmVec3SoundPos);
		//end 2010. 07. 07 by jskim 모선전시 사운드 버그 수정
	}
	//pTmSound->Set3DSoundPosition(&tmVec3SoundPos);
	
	if(nSType == SOUND_FLYING_M_A_GEAR 
		|| nSType == SOUND_FLYING_B_I_GEAR 
		|| nSType == SOUND_HIGH_BOOSTER 
		|| nSType == SOUND_LOW_BOOSTER
		|| nSType == SOUND_GROUND_MOVING_A_GEAR)
//		|| nSType == SOUND_MOVING_ON_WATER)
	{
		if(FAILED(pTmSound->Play(0, DSBPLAY_LOOPING, g_pSOption->sSoundVolume)))
		{
			DBGOUT(STRERR_C_SOUND_0003, nSType);
			return;
		}
	}
	else
	{
		if(FAILED(pTmSound->Play(0, 0, g_pSOption->sSoundVolume)))
		{
			return;
		}
	}
}

VOID CAtumSound::StopD3DSound(int nType)
{
	// 2013-02-05 by bhsohn Sound 없는 장비에서 Exception오류 나는 현상 수정
	if(!m_bNotDevice)
	{
		return;
	}
	// END 2013-02-05 by bhsohn Sound 없는 장비에서 Exception오류 나는 현상 수정

	FLOG("CAtumSound::StopD3DSound(int nType)");
	CSound *pTmSound = FindSCSoundWithSoundType(nType);
	if(pTmSound)
	{
		pTmSound->Stop(); // 2016-01-08 exception at this point (2)
	}	
}

VOID CAtumSound::StopD3DSound(char * str)
{
	// 2013-02-05 by bhsohn Sound 없는 장비에서 Exception오류 나는 현상 수정
	if(!m_bNotDevice)
	{
		return;
	}
	// END 2013-02-05 by bhsohn Sound 없는 장비에서 Exception오류 나는 현상 수정

	FLOG("CAtumSound::StopD3DSound(char * str)");
	int nType;
	nType = atoi(str);

	CSound *pTmSound = FindSCSoundWithSoundType(nType);
	if(pTmSound)
	{
		pTmSound->Stop();
	}
}

///////////////////////////////////////////////////////////////////////////////
/// \fn			void CAtumSound::DelD3DSound(int nType)
/// \brief		사운드 삭제
/// \author		ispark
/// \date		2006-09-07 ~ 2006-09-07
/// \warning	
///
/// \param		
/// \return		
///////////////////////////////////////////////////////////////////////////////
void CAtumSound::DelD3DSound(int nType)
{
	// 2013-02-05 by bhsohn Sound 없는 장비에서 Exception오류 나는 현상 수정
	if(!m_bNotDevice)
	{
		return;
	}
// END 2013-02-05 by bhsohn Sound 없는 장비에서 Exception오류 나는 현상 수정

	CSound *pTmSound = FindSCSoundWithSoundType(nType);
	if(pTmSound)
	{
		pTmSound->Stop();
	}

	DelSCSoundWithSoundType(nType);
}

VOID CAtumSound::CheckD3DSoundGround()
{
	// 2013-02-05 by bhsohn Sound 없는 장비에서 Exception오류 나는 현상 수정
	if(!m_bNotDevice)
	{
		return;
	}
// END 2013-02-05 by bhsohn Sound 없는 장비에서 Exception오류 나는 현상 수정

	FLOG("CAtumSound::CheckD3DSoundGround()");
	int type,type2;
	type = RANDI(0, 4);
	switch(type)
	{
	case 0:
		{
			type2 = RANDI(0, MAX_SOUND_GROUND_BIRD - 1);
			type2 += 100000;
		}
		break;
	case 1:
		{
			type2 = RANDI(0, MAX_SOUND_GROUND_WORM - 1);
			type2 += 100100;
		}
		break;
	case 2:
		{
			type2 = RANDI(0, MAX_SOUND_GROUND_ANIMAL - 1);
			type2 += 100200;
		}
		break;
	case 3:
		{
			type2 = RANDI(0, MAX_SOUND_GROUND_WATER - 1);
			type2 += 100300;
		}
		break;
	case 4:
		{
			type2 = RANDI(0, MAX_SOUND_GROUND_WIND - 1);
			type2 += 100400;
		}
	case 5:
		{
			type2 = RANDI(0, MAX_SOUND_GROUND_EXPLODE - 1);
			type2 += 100500;
		}
	case 6:
		{
			type2 = RANDI(0, MAX_SOUND_GROUND_MACHINE - 1);
			type2 += 100600;
		}
	case 7:
		{
			type2 = RANDI(0, MAX_SOUND_GROUND_HUMAN - 1);
			type2 += 100700;
		}
		break;
	}
//	DBGOUT("GroundSound Crate : [ %08d ]\n",type2);
//	PlayD3DSound(type2,m_pShuttleChild->m_vPos);
}


VOID CAtumSound::PlayBackSound()
{
	FLOG("CAtumSound::PlayBackSound()");
	// 2006-09-15 by ispark, 장치 활성 유무
	if(m_bNotDevice == FALSE)
	{
		return;
	}

	// 2006-04-20 by ispark, 시간제 배경음악
	if(m_bOnlyBackMusic == TRUE)
	{
		m_fSetSoundGroundCheckTime -= g_pD3dApp->GetElapsedTime();
		if(m_fSetSoundGroundCheckTime <= 0.0f)
		{
			m_bOnlyBackMusic = FALSE;
			m_nOnlyBackMusicState = -1;
			SetBackMusic(_GAME);
		}
	}
	
	if(!m_bPlayMusic)
	{
		if(m_pMusic)
			m_pMusic->Atum_MusicStop();
		if(strlen(m_strBackMusic))
		{
			char buf[128];
			wsprintf(buf,".\\Res-Snd\\%s",m_strBackMusic);
			m_pMusic->Atum_MusicInit(buf);
			int nVolume = -10000;
			if(m_nMusicVolume > 0)
			{
				nVolume = GetMusicVolume(m_nMusicVolume);
			}
			m_pMusic->Atum_PlayMP3(nVolume);
//			m_bPlayMusic = TRUE;
//			if(!FAILED(m_pMusic->Atum_MusicInit(buf)))
//			{
//				m_pMusic->Atum_PlayMP3(g_pSOption->sMusicVolume);
//				m_bPlayMusic = TRUE;
//			}
		}
	}
	// Sound
	if(m_bPlayMusic && m_pMusic)
		m_pMusic->Atum_LoopMusic();
	m_bPlayMusic = TRUE;

}

///////////////////////////////////////////////////////////////////////////////
/// \fn			
/// \brief		나레이션 플레이 
/// \author		// 2007-07-24 by bhsohn 나레이션 mp3추가
/// \date		2007-07-24 ~ 2007-07-24
/// \warning	
///
/// \param		
/// \return		
///////////////////////////////////////////////////////////////////////////////
BOOL CAtumSound::PlayNarrationSound(char* pNarration)
{
	// 2009. 01. 22 by ckPark 각 세력별 오퍼레이터 MP3 분리
	BOOL bSucSound = TRUE;
	// end 2009. 01. 22 by ckPark 각 세력별 오퍼레이터 MP3 분리
	FLOG("CAtumSound::PlayBackSound()");
	if(strlen(pNarration) <= 0)
	{
		// 2009. 01. 22 by ckPark 각 세력별 오퍼레이터 MP3 분리
		return bSucSound;
	}
	// 장치 활성 유무
	if(m_bNotDevice == FALSE)
	{
		// 2009. 01. 22 by ckPark 각 세력별 오퍼레이터 MP3 분리
		return bSucSound;
	}
	if(0 != strncmp(m_strNarrationMusic, pNarration, strlen(pNarration)+1))
	{
		m_bPlayNarrationMusic = FALSE;
		strncpy(m_strNarrationMusic, pNarration, strlen(pNarration)+1);
	}
	
	if(!m_bPlayNarrationMusic)
	{
		if(m_pNarrationMusic)
			m_pNarrationMusic->Atum_MusicStop();
		if(strlen(pNarration))
		{
			char buf[128];
			wsprintf(buf,".\\Res-Snd\\%s.mp3", pNarration);
			// 2009. 01. 22 by ckPark 각 세력별 오퍼레이터 MP3 분리
			//m_pNarrationMusic->Atum_MusicInit(buf);
			if(FAILED(m_pNarrationMusic->Atum_MusicInit(buf)))
			{
				bSucSound = FALSE;
			}
			// end 2009. 01. 22 by ckPark 각 세력별 오퍼레이터 MP3 분리
			
			// 2010. 07. 07 by jskim 모선전시 사운드 버그 수정 - 나레이션 볼륨을 옵션 볼륨으로 변경 
			int nVolume = -10000;
 			int nMusicVolume = NARRATION_VOLUME; // 나레이션 볼륨은 50으로 지정 
 			if(nMusicVolume > 0 && g_pSOption->sSoundVolume != -10000)
 			{
 				nVolume = GetMusicVolume(nMusicVolume);
 			}
			//end 2010. 07. 07 by jskim 모선전시 사운드 버그 수정 - 나레이션 볼륨을 옵션 볼륨으로 변경 

			m_pNarrationMusic->Atum_PlayMP3(nVolume);
		}
	}
	// Sound
	if(m_bPlayNarrationMusic && m_pNarrationMusic)
	{
		m_pNarrationMusic->Atum_LoopMusic();
	}	

	// 파일이 끝났다. 
	if(m_bPlayNarrationMusic && (FALSE == m_pNarrationMusic->IsNowPlay()))
	{		
		g_pD3dApp->EndNarrationSound();		
	}
	m_bPlayNarrationMusic = TRUE;

	// 2009. 01. 22 by ckPark 각 세력별 오퍼레이터 MP3 분리
	return bSucSound;
	// end 2009. 01. 22 by ckPark 각 세력별 오퍼레이터 MP3 분리
}

///////////////////////////////////////////////////////////////////////////////
/// \fn			
/// \brief		나레이션 플레이 종료  
/// \author		// 2007-07-24 by bhsohn 나레이션 mp3추가
/// \date		2007-07-24 ~ 2007-07-24
/// \warning	
///
/// \param		
/// \return		
///////////////////////////////////////////////////////////////////////////////
void CAtumSound::EndNarrationSound()
{
	// 2013-02-05 bhsohn Sound Exception
	if (!m_bNotDevice) return;

	if (m_pNarrationMusic) m_pNarrationMusic->Atum_MusicStop();
	
	m_bPlayNarrationMusic = false;

	util::zero(m_strNarrationMusic);
}

void CAtumSound::SetBackMusic(DWORD dwType)
{
	// 2013-02-05 bhsohn Sound Exception
	if (!m_bNotDevice) return;
	
	FLOG("CAtumSound::SetBackMusic(DWORD dwType)");

	switch(dwType)
	{
		// 1. LOGO & INIT
		// 2. SELECT & CREATE
		// 3. OPTION & MAPLOAD
		// 4. GAME & SHOP & CITY
		// 5. SCRIPT
		// 6. WAITING
		// 7. GAMEOVER & COMPLETE & PROLOGUE
		// 8. ENDGAME
		// 9. ENDCLIENT
		//10. BOSSMONSTERSUMMONREADY
		//11. BOSSMONSTERSUMMON
		//12. SERVERDOWN (C_SERVER_DOWN_ALARM not defined)

	case _LOGO:
	case _INIT:

		m_bPlayMusic = false;

		util::zero(m_strBackMusic);
			
		// 2007-07-24 by bhsohn 나레이션 mp3추가
		g_pD3dApp->EndNarrationSound();

		m_nMusicVolume = OPTION_DEFAULT_MUSICVOLUME;

		break;

	case _SELECT:
	case _CREATE:

		if (g_pD3dApp->m_dwGameState != _SELECT && g_pD3dApp->m_dwGameState != _CREATE)
		{
			if (strcmp(m_strBackMusic,"BGM_SELECT.mp3") != 0)
			{
				m_bPlayMusic = false;
				strcpy(m_strBackMusic,"BGM_SELECT.mp3");
			}
		}

		m_nMusicVolume = OPTION_DEFAULT_MUSICVOLUME;

		break;

	case _OPTION:
	case _MAPLOAD:

		if (g_pGameMain && !g_pGameMain->m_pMp3Player->GetStopButton())
				
			m_bPlayMusic = false;

		util::zero(m_strBackMusic);
			
		g_pD3dApp->EndNarrationSound();
		
		break;

	case _GAME:
	case _SHOP:
	case _CITY:

		// todo : review this part
		if((g_pD3dApp->m_dwGameState != _GAME &&
			g_pD3dApp->m_dwGameState != _SHOP &&
			g_pD3dApp->m_dwGameState != _CITY &&
			m_nOnlyBackMusicState != _BOSSMONSTERSUMMONREADY &&
			m_nOnlyBackMusicState != _BOSSMONSTERSUMMON &&
			m_nOnlyBackMusicState != _SERVER_DOWN ) || // 2013-07-05 by bhsohn 서버종료시, 경고음 시스템
			m_nOnlyBackMusicState == -1)
		{
			if(g_pD3dApp->m_pShuttleChild)
			{
				char buf[64];
				// 2007-08-02 by dgwoo 음악 파일은 맵번호가아닌 bgm을 읽어서 로딩한다.
				//wsprintf( buf, "BGM_%d.mp3", g_pShuttleChild->m_myShuttleInfo.MapChannelIndex.MapIndex);
				wsprintf( buf, "BGM_%d.mp3", GetMapIndexBGM(g_pShuttleChild->m_myShuttleInfo.MapChannelIndex.MapIndex));
				if(strcmp(m_strBackMusic,buf))
				{
//						if(g_pGameMain->m_pMp3Player->m_vecMp3FileNames.size())
//						{// mp3 파일이 있을경우엔 기존의 배경음악을 그대로 유지한다.
//								
//						}
//						else
					{// mp3 파일이 없을경우 진입한 배경음악으로 변경.
						if(g_pGameMain != NULL && g_pGameMain->m_pMp3Player != NULL &&
							!g_pGameMain->m_pMp3Player->GetStopButton())
						{// 정지 버튼을 누른 상태라면 다른 맵에서도 배경음악을 켜지지 않는다.
							m_bPlayMusic = FALSE;
						}
						strcpy(m_strBackMusic,buf);
					}
				}
				else
				{
// 2006-05-15 by ispark, 상점 이용시 기본 배경음악이 꺼지는 것을 방지 하기 위해서 지웠다.
//						m_bPlayMusic = FALSE;
//						memset(m_strBackMusic,0x00,sizeof(m_strBackMusic));
				}

				// 2007-08-01 by bhsohn 튜토리얼 맵에서 음성 플레이 시도할려는 버그 처리
				if(IS_TUTORIAL_MAP_INDEX(g_pShuttleChild->m_myShuttleInfo.MapChannelIndex.MapIndex))
				{
					m_bPlayMusic = FALSE;
					memset(m_strBackMusic,0x00,sizeof(m_strBackMusic));
				}
				// end 2007-08-01 by bhsohn 튜토리얼 맵에서 음성 플레이 시도할려는 버그 처리
			}
			else
			{
				m_bPlayMusic = FALSE;
				memset(m_strBackMusic,0x00,sizeof(m_strBackMusic));

				// 2007-07-24 by bhsohn 나레이션 mp3추가
				g_pD3dApp->EndNarrationSound();
			}
		}
		
		else if (m_nOnlyBackMusicState > 0)
			
			SetBackMusic(m_nOnlyBackMusicState);
		
		m_nMusicVolume = g_pSOption->sMusicVolume;
		
		break;

	case _SCRIPT:
		
		m_bPlayMusic = false;

		util::zero(m_strBackMusic);

		g_pD3dApp->EndNarrationSound();
		
		break;

	case _WAITING:

		if (g_pD3dApp->m_bClientQuit)
		{
			if(strcmp(m_strBackMusic,"BGM_ENDGAME.mp3"))
			{
				m_bPlayMusic = false;
				strcpy(m_strBackMusic,"BGM_ENDGAME.mp3");
			}
		}

		m_nMusicVolume = OPTION_DEFAULT_MUSICVOLUME;

		break;

	case _GAMEOVER:
	case _COMPLETE:
	case _PROLOGUE:

		m_bPlayMusic = false;

		util::zero(m_strBackMusic);

		g_pD3dApp->EndNarrationSound();

		break;

	// todo : panoskj i think this could be the place to turn off
	// todo : missile warnings when you close the game

	case _ENDGAME:

		break;

	case _ENDCLIENT:

		break;

	case _BOSSMONSTERSUMMONREADY:

		if (g_pD3dApp->m_pShuttleChild)
		{
			char buf[32] { };

			snprintf(buf, 32, "BGM_%d.mp3", 3002);

			if (strcmp(m_strBackMusic, buf) != 0)
			{
				// 2013-04-11 ssjung
				if (!g_pGameMain->m_pMp3Player->GetStopButton())
					m_bPlayMusic = false;

				strcpy(m_strBackMusic, buf);

				m_nOnlyBackMusicState = _BOSSMONSTERSUMMONREADY;
			}
		}
		
		break;

		// 2006-04-20 ispark
	case _BOSSMONSTERSUMMON:

		if(g_pD3dApp->m_pShuttleChild)
		{
			auto buf = "BGM_WARNING.mp3";

			if (strcmp(m_strBackMusic, buf) != 0)
			{
				m_bPlayMusic = false;
				strcpy(m_strBackMusic, buf);
				m_nOnlyBackMusicState = _BOSSMONSTERSUMMON;
				m_fSetSoundGroundCheckTime = 60.0f;
				m_bOnlyBackMusic = true;
			}
		}

		break;

		// 2013-07-05 bhsohn
	case _SERVER_DOWN:

#ifdef C_SERVER_DOWN_ALARM
		if (g_pD3dApp->m_pShuttleChild)
		{
			auto buf = "BGM_WARNING.mp3";

			if (strcmp(m_strBackMusic, buf) != 0)
			{					
				m_bPlayMusic = false;
				strcpy(m_strBackMusic,buf);
				m_nOnlyBackMusicState = _SERVER_DOWN;
				m_bOnlyBackMusic = false;
			}
		}
#endif
		break;
	}
}

void CAtumSound::SetAtumMusicVolume(int nVolume) const
{
	// 2013-02-05 by bhsohn Sound Exception
	if (!m_bNotDevice) return;
	
	m_pMusic->SetAtumMusicVolume(nVolume);
}

CSound* CAtumSound::FindSCSoundWithSoundType(int nType) const
{
	auto r = find_if(m_vectorSoundPtr.cbegin() , m_vectorSoundPtr.cend(),

		[nType](CSound* snd) { return snd->m_nSoundType == nType; });

	return r == m_vectorSoundPtr.cend() ? nullptr : *r;
}

void CAtumSound::DelSCSoundWithSoundType(int nType)
{
	auto r = find_if(m_vectorSoundPtr.begin(), m_vectorSoundPtr.end(),

		[nType](CSound* snd) { return snd->m_nSoundType == nType; });

	if (r != m_vectorSoundPtr.end()) m_vectorSoundPtr.erase(r);
}


void CAtumSound::SetDefaultMusicLoop(bool bFlag) const
{
	m_pMusic->m_bDefaultMusic = bFlag;
}

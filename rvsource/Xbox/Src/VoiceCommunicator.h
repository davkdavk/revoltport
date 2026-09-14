//-----------------------------------------------------------------------------
// File: VoiceCommunicator.h
//
// Desc: Class and strucuture definitions related to VoiceCommunicator support
//
// Hist: 1.17.02 - Created
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef VOICECOMMUNICATOR_H
#define VOICECOMMUNICATOR_H

#include <xtl.h>
#ifdef _XBOX360
// Voice chat moves to XHV/GameChat on 360 (deferred with Live). Stub the OG
// XVoice media-object types so dependents parse; the .cpp gets a real port.
struct IXVoiceEncoder;
struct IXVoiceDecoder;
typedef IXVoiceEncoder *LPXVOICEENCODER;
typedef IXVoiceDecoder *LPXVOICEDECODER;
typedef DWORD XVOICE_MASK;
struct XMEDIAPACKET;
struct XMediaObject;
#else
#include <xvoice.h>
#endif
#include <xonline.h>

class CVoiceManager;

class CVoiceCommunicator
{
public:
    friend CVoiceManager;

    CVoiceCommunicator();
    ~CVoiceCommunicator();

    HRESULT Initialize( CVoiceManager* pManager );
    HRESULT Shutdown();

    HRESULT ResetMicrophone();
    HRESULT ResetHeadphone();
    HRESULT OnInsertion( DWORD dwSlot );
    HRESULT OnRemoval();

    DWORD   MicrophonePacketStatus();
    HRESULT GetMicrophonePacket( XMEDIAPACKET* pPacket );
    HRESULT SubmitMicrophonePacket( XMEDIAPACKET* pPacket );

    DWORD   HeadphonePacketStatus();
    HRESULT GetHeadphonePacket( XMEDIAPACKET* pPacket );
    HRESULT SubmitHeadphonePacket( XMEDIAPACKET* pPacket );

private:
    CVoiceManager*  m_pManager;
    LONG            m_lSlot;
    XMediaObject*   m_pMicrophoneXMO;
    XMediaObject*   m_pHeadphoneXMO;
    LPXVOICEENCODER m_pEncoderXMO;
    LPXVOICEDECODER m_pDecoderXMO;

    DWORD           m_dwSRCReadPosition;
    DWORD           m_dwRampTime;

    DWORD*          m_adwMicrophonePacketStatus;
    DWORD           m_dwNextMicrophonePacket;
    BYTE*           m_pbMicrophoneBuffer;

    DWORD*          m_adwHeadphonePacketStatus;
    DWORD           m_dwNextHeadphonePacket;
    BYTE*           m_pbHeadphoneBuffer;
};


#endif // VOICECOMMUNICATOR_H
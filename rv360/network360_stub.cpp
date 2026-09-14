#include "../rvsource/Xbox/Src/revolt.h"
#include "../rvsource/Xbox/Src/network.h"

#ifdef _XBOX360

// Offline-first network implementation. It deliberately reports no session,
// remote players, or pending messages. This is sufficient for single-player
// linking and keeps multiplayer failure explicit until the system-link rewrite.

WORD LocalPlayerID = 0;
DWORD FromID = 0, ServerID = 0;
char SendMsgBuffer[PACKET_BUFFER_SIZE];
char *RecvMsgBuffer = NULL;
long MessageQueueSize = 0;
NET_PLAYER PlayerList[MAX_NUM_PLAYERS];
long PlayerCount = 0;
PLAYER_LIST DepartedPlayerList;
NET_MACHINE MachineList[MAX_NUM_MACHINES];
long MachineCount = 0;
NET_MACHINE *pMachineLocal = NULL;
NET_SESSION SessionList[MAX_SESSION_SEARCH_RESULTS];
static char SessionCount = 0;
bool DisableSessionListUpdate = false;
NET_SESSION SessionCurr;
SOCKET soUDP = INVALID_SOCKET;
XNADDR XnAddrLocal;
BOOL bGameStarted = FALSE;
long NextPositionReady = 0, NextPacketReady = 0, NextSyncReady = 0;
long AllPlayersReady = 0, HostQuit = 0, LocalPlayerReady = 0;
REAL NextPacketTimer = ZERO, NextSyncTimer = ZERO, NextPositionTimer = ZERO;
REAL AllReadyTimeout = ZERO;
long NextSyncMachine = 0;
char SessionPick = 0;
DWORD dwLocalPlayerCount = 1;
BYTE g_bInvitedByFriend = 0;
LONG g_lMatchmakingLevelNum = -1;
const ZERO_UNION g_Zero = { 0 };

bool InitNetwork(void) { return true; }
void KillNetwork(void) {}
int GetRemoteMessages(void) { return 0; }
bool CreateSession(void) { return false; }
void DestroySession(void) {}
BOOL JoinSession(int) { return FALSE; }
void LeaveSession(void) {}
void RequestSessionList(void) {}
void InsertSessionListEntry(NET_SESSION*) {}
void DeleteSessionListEntry(int) {}
void ClearSessionList(void) { SessionCount = 0; }
char GetSessionCount(void) { return SessionCount; }
void SetGameStarted(void) { bGameStarted = TRUE; }
void QueueMessage(void*, WORD) {}
void QueueVoiceMessage(void*, WORD, DWORD) {}
void TransmitMessageQueue(void) {}
void RequestAddPlayers(DWORD) {}
BYTE GetUnusedPlayerID(void) { return 0; }
LONG PlayerIndexFromPlayerID(DWORD) { return -1; }
LONG PlayerIndexFromXUID(XUID) { return -1; }
void LookForClientConnections(void) {}
void CreateLocalServerPlayers(void) {}
void ProcessMessage(void) {}
int ProcessVoiceMessage(void) { return 0; }
void SendVoiceInfoMessage(DWORD, DWORD) {}
int ProcessVoiceInfoMessage(void) { return 0; }
void SetPlayerData(void) {}
unsigned long GetSendQueueLength(void) { return 0; }
void UpdatePacketInfo(void) {}
void CheckAllPlayersReady(void) {}
void SendGameStarted(void) {}
int ProcessGameStarted(void) { return 0; }
void SendSyncRequest(void) {}
int ProcessSyncRequest(void) { return 0; }
int ProcessSyncReply(void) { return 0; }
void SendGameLoaded(void) {}
int ProcessGameLoaded(void) { return 0; }
void SendCountdownStart(void) {}
int ProcessCountdownStart(void) { return 0; }
void SendRaceFinishTime(void) {}
int ProcessRaceFinishTime(void) { return 0; }
void SendPlayerSync(void) {}
int ProcessPlayerSync1(void) { return 0; }
int ProcessPlayerSync2(void) { return 0; }
int ProcessPlayerSync3(void) { return 0; }
void SendMultiplayerRestart(void) {}
int ProcessMultiplayerRestart(void) { return 0; }
void ClientMultiplayerRestart(void) {}
void SendPosition(void) {}
int ProcessPosition(void) { return 0; }
void SendTransferBomb(GLOBAL_ID, unsigned long, unsigned long) {}
int ProcessTransferBomb(void) { return 0; }
void SendTransferFox(unsigned long, unsigned long) {}
int ProcessTransferFox(void) { return 0; }
void SendBombTagClock(void) {}
int ProcessBombTagClock(void) { return 0; }
void SendElectroPulseTheWorld(long) {}
int ProcessElectroPulseTheWorld(void) { return 0; }
void SendGotGlobal(void) {}
int ProcessGotGlobal(void) { return 0; }
void SendHonka(void) {}
int ProcessHonka(void) { return 0; }
void RemoteSyncHost(void) {}
void RemoteSyncClient(void) {}
void ProcessPlayerJoining(void) {}
void ProcessPlayerLeaving(void) {}
#ifndef XBOX_NOT_YET_IMPLEMENTED
void ProcessBecomeHost(void) {}
#endif

#endif

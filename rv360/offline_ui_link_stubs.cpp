#include "../rvsource/Xbox/Src/revolt.h"
#include "../rvsource/Xbox/Src/ui_StateEngine.h"
#include "../rvsource/Xbox/Src/ui_LiveSignOn.h"
#include "../rvsource/Xbox/Src/ui_ContentDownload.h"

#ifdef _XBOX360

CLiveSignInStateEngine g_LiveSignInStateEngine;
CLiveSignOutStateEngine g_LiveSignOutStateEngine;
CRequiredDownloadEngine g_RequiredDownloadEngine;
COptionalDownloadEngine g_OptionalDownloadEngine;

VOID CheckSignInConditions() {}
VOID CLiveSignInStateEngine::HandleEnterFromParent() {}
VOID CLiveSignInStateEngine::HandleExitToParent() {}
VOID CLiveSignOutStateEngine::HandleEnterFromParent() {}
VOID CLiveSignOutStateEngine::HandleExitToParent() {}
BOOL CLiveSignInStateEngine::PlayersSignedIn() { return FALSE; }
HRESULT CLiveSignInStateEngine::SignOut() { return S_OK; }
HRESULT CLiveSignInStateEngine::Process()
{
    Return(STATEENGINE_TERMINATED);
    return S_OK;
}
HRESULT CLiveSignOutStateEngine::Process()
{
    Return(STATEENGINE_TERMINATED);
    return S_OK;
}
VOID CContentDownloadEngine::Return(DWORD status) { CUIStateEngine::Return(status); }
VOID CContentDownloadEngine::BeginError(const WCHAR*, const WCHAR*) {}
VOID CContentDownloadEngine::BeginSuccess(const WCHAR*, const WCHAR*) {}
HRESULT CRequiredDownloadEngine::Process()
{
    Return(STATEENGINE_TERMINATED);
    return S_OK;
}
HRESULT COptionalDownloadEngine::Process()
{
    Return(STATEENGINE_TERMINATED);
    return S_OK;
}

#endif

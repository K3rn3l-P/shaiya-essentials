#include "include/shaiya/include/CWorldMgr.h"
using namespace shaiya;

CCharacter* CWorldMgr::FindUser(uint objectId)
{
    typedef CCharacter* (__thiscall* LPFN)(void*, uint);
    return (*(LPFN)0x452B10)((void*)0x7C4A68, objectId);
}

CMonster* CWorldMgr::FindMob(uint objectId)
{
    typedef CMonster* (__thiscall* LPFN)(void*, ULONG);
    return (*(LPFN)0x452B90)((void*)0x7C4A68, id);
}

CNpc* CWorldMgr::FindNpc(ULONG id/*CNpc->id*/)
{
    typedef CNpc* (__thiscall* LPFN)(void*, ULONG);
    return (*(LPFN)0x452C00)((void*)0x7C4A68, id);
}

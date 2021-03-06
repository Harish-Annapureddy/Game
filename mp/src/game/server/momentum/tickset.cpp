#include "cbase.h"
#include "tickset.h"
#include "util/engine_patch.h"
#include "mom_shareddefs.h"
#include "tier0/platform.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static void OnIntervalPerTickChange(IConVar *var, const char *pOldValue, float fOldValue)
{
    ConVarRef tr(var);
    float tickrate = tr.GetFloat();
    if (CloseEnough(tickrate, TickSet::GetTickrate(), FLT_EPSILON))
        return;

    TickSet::SetTickrate(tickrate);
}

static void OnTickRateSet(const CCommand &command)
{
    // Search defined rates for one with a string matching the command argument.
    for (unsigned rateIndx = TickSet::TICKRATE_FIRST; rateIndx < TickSet::TICKRATE_COUNT; rateIndx++)
    {
        if (FStrEq(command.ArgS(), TickSet::s_DefinedRates[rateIndx].sType))
        {
            sv_interval_per_tick.SetValue(TickSet::s_DefinedRates[rateIndx].fTickRate);
            return;
        }
    }
    Warning("Unknown tickrate. Use \"sv_interval_per_tick\" to set custom tickrates.");
}

static int OnTickRateAutocomplete(const char *partial,
                                  char commands[COMMAND_COMPLETION_MAXITEMS][COMMAND_COMPLETION_ITEM_LENGTH])
{
    const int commandLength = Q_strlen("sv_tickrate ");

    if (Q_strlen(partial) < commandLength)
    {
        // Checking for circumstances where partial is not as long as expected.
        partial = "";
    }
    else
    {
        // Move start of the string to expected start of parameter.
        partial += commandLength;
    }

    const unsigned partialLength = Q_strlen(partial);
    int suggestionCount = 0;

    // Search defined rates for one with a string matching the command argument.
    for (unsigned rateIndx = TickSet::TICKRATE_FIRST; rateIndx < TickSet::TICKRATE_COUNT && suggestionCount < COMMAND_COMPLETION_MAXITEMS; rateIndx++)
    {
        if (!Q_strncmp(partial, TickSet::s_DefinedRates[rateIndx].sType, partialLength))
        {
            Q_snprintf(commands[suggestionCount], COMMAND_COMPLETION_ITEM_LENGTH, "sv_tickrate %s", TickSet::s_DefinedRates[rateIndx].sType);
            ++suggestionCount;
        }
    }

    return suggestionCount;
}

MAKE_CONVAR_C(sv_interval_per_tick, "0.015", FCVAR_MAPPING,
    "Changes the interval per tick of the engine. Interval per tick is 1/tickrate, so 100 tickrate = 0.01.",
    0.001f, 0.1f, OnIntervalPerTickChange);

static ConCommand sv_tickrate("sv_tickrate", OnTickRateSet,
    "Changes the tickrate to one of a defined set of values. Custom tickrates can be set using \"sv_interval_per_tick.\"",
    FCVAR_MAPPING, OnTickRateAutocomplete);

float *TickSet::interval_per_tick = nullptr;

const Tickrate TickSet::s_DefinedRates[] = {
    { 0.015625f, "64" },
    { 0.015f, "66" },
    { 0.01171875f, "85" },
    { 0.01f, "100" },
    { 0.0078125f, "128" }
};

Tickrate TickSet::m_trCurrent = s_DefinedRates[TICKRATE_66];

bool TickSet::TickInit()
{
#ifdef _WIN32
    const char *pattern = "\x8B\x0D\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xFF\x00\xD9\x15\x00\x00\x00\x00\xDD\x05\x00\x00\x00\x00\xDB\xF1\xDD\x05";
    auto addr = reinterpret_cast<uintptr_t>(CEngineBinary::FindPattern(pattern, "xx????????????x?xx????xx????xxxx", 18));
    if (addr)
        interval_per_tick = *reinterpret_cast<float**>(addr);

#else //POSIX
#ifdef __linux__

    // mov ds:interval_per_tick, 3C75C28Fh         <-- float for 0.015
    const char *pattern = "\xC7\x05\x00\x00\x00\x00\x8F\xC2\x75\x3C\xE8";
    void* addr = CEngineBinary::FindPattern(pattern, "xx????xxxxx", 2);
    if (addr)
        interval_per_tick = *(float**)(addr); //MOM_TODO: fix pointer arithmetic on void pointer?

#elif defined (OSX)
    if (CEngineBinary::GetModuleSize() == 12581936) //magic engine.dylib file size as of august 2017
    {
        interval_per_tick = reinterpret_cast<float*>((char*)CEngineBinary::GetModuleBase() + 0x7DC120); //use offset since it's quicker than searching
        printf("engine.dylib not updated. using offset! address: %#08x\n", interval_per_tick);
    }
    else //valve updated engine, try to use search pattern...
    {
        const char *pattern = "\x8F\xC2\x75\x3C\x78\x00\x00\x0C\x6C\x00\x00\x00\x01\x00";
        auto addr = reinterpret_cast<uintptr_t>(CEngineBinary::FindPattern(pattern, "xxxxx??xx???xx"));
        if (addr)
        {
            interval_per_tick = reinterpret_cast<float*>(addr);
            printf("Found interval_per_tick using search! address: %#08x\n", interval_per_tick);
        }
    }
#endif //__linux__ or OSX
#endif //WIN32
    return interval_per_tick ? true : false;
}

bool TickSet::SetTickrate(float tickrate)
{
    if (!CloseEnough(m_trCurrent.fTickRate, tickrate, FLT_EPSILON))
    {
        Tickrate tr = {tickrate, "CUSTOM"};
        for (int tickRateIndx = TICKRATE_FIRST; tickRateIndx < TICKRATE_COUNT; tickRateIndx++)
        {
            if (CloseEnough(tickrate, s_DefinedRates[tickRateIndx].fTickRate, FLT_EPSILON))
            {
                tr = s_DefinedRates[tickRateIndx];
                break;
            }
        }
        return SetTickrate(tr);
    }
    
    return false;
}

bool TickSet::SetTickrate(Tickrate trNew)
{
    if (trNew == m_trCurrent)
    {
        DevLog("Tickrate not changed: new == current\n");
        return false;
    }

    if (interval_per_tick)
    {
        *interval_per_tick = trNew.fTickRate;
        gpGlobals->interval_per_tick = *interval_per_tick;
        m_trCurrent = trNew;
        auto pPlayer = UTIL_GetLocalPlayer();
        if (pPlayer)
        {
            engine->ClientCommand(pPlayer->edict(), "reload");
        }
        DevLog("Interval per tick set to %f\n", trNew.fTickRate);
        return true;
    }
    Warning("Failed to set tickrate: bad hook\n");
    return false;
}
#pragma once

class CBaseMomZoneTrigger;
class CMomentumPlayer;
class CMomBaseZoneBuilder;

class CMapZoneEdit
{
  public:
    CMapZoneEdit();

    bool IsEditing() const { return m_bEditing; }
    void StopEditing();

    void LevelInit();
    void LevelShutdown();
    void FrameUpdate();

    void OnCreate(int zonetype = -1);
    void OnMark();
    void OnRemove();
    void OnCancel();

    bool GetCurrentBuildSpot(CMomentumPlayer *pPlayer, Vector &vecPos);

    void IncreaseZoom(float dist) { m_flReticleDist = fminf(m_flReticleDist + dist, 2048.0f); }
    void DecreaseZoom(float dist) { m_flReticleDist = fmaxf(m_flReticleDist - dist, 16.0f); }

    int GetEntityZoneType(CBaseEntity *pEnt);
    CBaseMomZoneTrigger *CreateZoneEntity(int type);
    void SetZoneProps(CBaseMomZoneTrigger *pEnt);
    int ShortNameToZoneType(const char *in);

    CMomBaseZoneBuilder *GetBuilder();
    void SetBuilder(CMomBaseZoneBuilder *pNewBuilder);
    void ResetBuilder();
    CMomentumPlayer *GetPlayerBuilder() const;

  private:
    bool m_bEditing;

    float m_flReticleDist;

    CMomBaseZoneBuilder *m_pBuilder;
};
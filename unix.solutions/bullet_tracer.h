#pragma once
#include "../sdk/structs.h"
#include <algorithm>
#include <cmath>
#include <mutex>
#include <vector>
#include <windows.h>

namespace Hooks {
extern bool g_bBulletTracer;
extern float g_fTracerLife;
extern float g_fTracerThickness;
} // namespace Hooks

namespace Features {
namespace Visuals {
struct Trace {
  Vector3 startPos;
  Vector3 endPos;
  float spawnTime;
  float totalDist;
};

inline std::vector<Trace> traces;
inline std::mutex traceMutex;
inline int lastShotsFired = -1;
inline float bulletSpeed = 8000.0f;

inline void AngleToDirection(const Vector3 &angles, Vector3 &dir) {
  float pitch = angles.x * (3.14159265f / 180.0f);
  float yaw = angles.y * (3.14159265f / 180.0f);
  float cp = cosf(pitch);
  float sp = sinf(pitch);
  float cy = cosf(yaw);
  float sy = sinf(yaw);
  dir.x = cp * cy;
  dir.y = cp * sy;
  dir.z = -sp;
}

inline void AddTrace(const Vector3 &eyePos, const Vector3 &angles) {
  Vector3 dir;
  AngleToDirection(angles, dir);

  Trace t;
  t.startPos = eyePos;
  t.endPos = {eyePos.x + dir.x * 8000.0f, eyePos.y + dir.y * 8000.0f,
              eyePos.z + dir.z * 8000.0f};
  t.spawnTime = (float)GetTickCount64() / 1000.0f;

  float dx = t.endPos.x - t.startPos.x;
  float dy = t.endPos.y - t.startPos.y;
  float dz = t.endPos.z - t.startPos.z;
  t.totalDist = sqrtf(dx * dx + dy * dy + dz * dz);

  std::lock_guard<std::mutex> lock(traceMutex);
  traces.push_back(t);
}

inline void Update(uintptr_t lp, const Vector3 &angles) {
  if (!Hooks::g_bBulletTracer || !lp)
    return;

  int currentShots = *(int *)(lp + 0x270C); // m_iShotsFired
  if (currentShots > lastShotsFired && lastShotsFired >= 0) {
    // Better Eye Position calculation
    uintptr_t sceneNode = *(uintptr_t *)(lp + 0x338); // m_pGameSceneNode
    if (sceneNode) {
      Vector3 origin = *(Vector3 *)(sceneNode + 0xD0); // m_vecAbsOrigin
      Vector3 viewOffset = *(Vector3 *)(lp + 0xD58);   // m_vecViewOffset
      AddTrace({origin.x + viewOffset.x, origin.y + viewOffset.y,
                origin.z + viewOffset.z},
               angles);
    }
  }
  lastShotsFired = currentShots;
}

inline void Render(ImDrawList *drawList, const view_matrix_t &viewMatrix,
                   int screenWidth, int screenHeight) {
  if (!Hooks::g_bBulletTracer)
    return;

  float now = (float)GetTickCount64() / 1000.0f;
  std::lock_guard<std::mutex> lock(traceMutex);

  // Cleanup expired traces
  traces.erase(std::remove_if(traces.begin(), traces.end(),
                              [now](const Trace &t) {
                                return (now - t.spawnTime) >
                                       Hooks::g_fTracerLife;
                              }),
               traces.end());

  for (const auto &t : traces) {
    float age = now - t.spawnTime;
    float travelTime = t.totalDist / bulletSpeed;
    if (travelTime < 0.01f)
      travelTime = 0.01f;
    float bulletFrac = age / travelTime;
    if (bulletFrac > 1.0f)
      bulletFrac = 1.0f;

    float trailAge = age - travelTime;
    float trailAlpha = 1.0f;
    if (trailAge > 0.0f) {
      trailAlpha = 1.0f - (trailAge / Hooks::g_fTracerLife);
      if (trailAlpha <= 0.0f)
        continue;
    }

    // Segmented Rendering (from unix.solutions)
    constexpr int SEGMENTS = 12;
    Vector2 pts[SEGMENTS + 1];
    bool ok[SEGMENTS + 1] = {};

    for (int s = 0; s <= SEGMENTS; s++) {
      float segFrac = ((float)s / (float)SEGMENTS) * bulletFrac;
      Vector3 pos3d = {t.startPos.x + (t.endPos.x - t.startPos.x) * segFrac,
                       t.startPos.y + (t.endPos.y - t.startPos.y) * segFrac,
                       t.startPos.z + (t.endPos.z - t.startPos.z) * segFrac};
      ok[s] =
          WorldToScreen(pos3d, pts[s], viewMatrix, screenWidth, screenHeight);
    }

    for (int s = 0; s < SEGMENTS; s++) {
      if (!ok[s] || !ok[s + 1])
        continue;

      float segFrac = (float)s / (float)SEGMENTS;
      float brightness = 0.3f + 0.7f * segFrac;
      int alpha = (int)(trailAlpha * brightness * 255.0f);

      ImU32 col = IM_COL32(255, 255, 255, alpha);
      drawList->AddLine(ImVec2(pts[s].x, pts[s].y),
                        ImVec2(pts[s + 1].x, pts[s + 1].y), col,
                        Hooks::g_fTracerThickness);
    }
  }
}
} // namespace Visuals
} // namespace Features

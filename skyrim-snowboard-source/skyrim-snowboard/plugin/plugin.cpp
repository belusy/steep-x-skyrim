// Skyrim SE ASI plugin skeleton. UNTESTED: not compiled, never run in the game.
// Loaded by Ultimate ASI Loader (which Melty installs for Skyrim SE).
//
// SAFE BY DEFAULT: every game hook below is unverified (sheets/hooks.json has no addresses yet),
// so SkyrimGame reports "board not equipped" and the plugin does nothing to the game.
// Fill in the hooks one by one, flip kHooksVerified only after each is confirmed in the running game.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <atomic>
#include <cstdio>

#include "controller.h"

namespace {

constexpr bool kHooksVerified = false;  // flip only when ALL rows in sheets/hooks.json are verified

std::atomic<bool> g_running{true};
FILE* g_log = nullptr;

void log(const char* msg) {
    if (!g_log) return;
    std::fprintf(g_log, "%s\n", msg);
    std::fflush(g_log);
}

class SkyrimGame : public snow::IGame {
public:
    // TODO(hooks.json: equip_board): detect that the player has the board equipped.
    bool board_equipped() override { return kHooksVerified && false; }

    // TODO(hooks.json: read_input): poll keys from sheets/inputs.json (check against Skyrim bindings first).
    snow::Input read_input() override { return {}; }

    // TODO(hooks.json: read_ground, snow_surface): terrain height / slope / surface under the player.
    bool read_ground(snow::Ground&) override { return false; }

    // TODO(hooks.json: player_move_override): apply speed/heading/z to the player; release control when !riding.
    // Convert metres to game units with snow::WORLD.units_per_meter.
    void apply(const snow::Rider&, bool) override {}

    // TODO: on-screen score / trick name. Until then, log only.
    void notify(snow::Event e, const snow::Rider& r) override {
        char buf[96];
        std::snprintf(buf, sizeof buf, "event=%d score=%d combo=%d", static_cast<int>(e), r.score, r.combo);
        log(buf);
    }
};

DWORD WINAPI worker(LPVOID) {
    SkyrimGame game;
    snow::Controller controller(game);
    LARGE_INTEGER freq, last, now;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&last);
    while (g_running) {
        Sleep(16);
        QueryPerformanceCounter(&now);
        float dt = static_cast<float>(now.QuadPart - last.QuadPart) / static_cast<float>(freq.QuadPart);
        last = now;
        if (dt > 0.1f) dt = 0.1f;  // after a pause or loading screen
        controller.update(dt);
    }
    return 0;
}

}  // namespace

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(module);
        // TODO: log under {localappdata}/SkyrimSnowboard/ rather than the game folder.
        g_log = std::fopen("SkyrimSnowboard.log", "w");
        log(kHooksVerified ? "snowboard plugin loaded (hooks verified)" : "snowboard plugin loaded (hooks NOT verified, inactive)");
        CreateThread(nullptr, 0, worker, nullptr, 0, nullptr);
    } else if (reason == DLL_PROCESS_DETACH) {
        g_running = false;
    }
    return TRUE;
}

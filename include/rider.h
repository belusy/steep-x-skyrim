// Platform-independent snowboard physics and trick scoring.
// Pure logic: no Skyrim, no Windows. Unit-tested on Linux (see tests/test_rider.cpp).
// The in-game plugin feeds it Input + Ground each frame and applies the resulting Rider state.
#pragma once
#include <algorithm>
#include <cmath>

#include "generated_sheets.h"

namespace snow {

enum class Event { None, Jumped, Landed, Trick, Wipeout };

struct Input {
    float steer = 0.f;  // -1 left .. +1 right
    bool brake = false, jump = false, spin_left = false, spin_right = false, grab = false;
};

struct Ground {
    float height_m = 0.f;
    float slope_deg = 0.f;
    int surface = 0;  // index into SURFACES (SurfaceId); -1 = not snow (rock, grass, water, ...)
};

struct Rider {
    float speed = 0.f;  // m/s along heading
    float heading_deg = 0.f;
    float z_m = 0.f, vz = 0.f;
    bool grounded = true;
    float air_time = 0.f, spin_deg = 0.f, grab_time = 0.f;
    int score = 0, combo = 0;
    int last_trick_points = 0;
};

inline bool trick_matches(const TrickRow& t, const Rider& r) {
    if (r.air_time < t.min_air_time_s) return false;
    if (t.kind == TrickKind::Grab) return r.grab_time >= t.min_air_time_s;
    const bool left = t.input == InputId::spin_left;
    if (left != (r.spin_deg < 0.f)) return false;
    return std::fabs(r.spin_deg) >= t.rotation_deg - SCORING.landing_tolerance_deg;
}

inline Event land(Rider& r, const Ground& g) {
    r.z_m = g.height_m;
    r.vz = 0.f;
    r.grounded = true;
    r.last_trick_points = 0;

    if (r.air_time < SCORING.min_air_time_for_score_s) return Event::Landed;

    // Landing is clean when the board points forward or fully switch (multiples of 180).
    const float rem = std::fmod(std::fabs(r.spin_deg), 180.f);
    const float off = std::min(rem, 180.f - rem);
    if (off > SCORING.landing_tolerance_deg) {
        r.speed *= SCORING.wipeout_speed_keep;
        r.combo = 0;
        r.score = std::max(0, r.score - SCORING.wipeout_penalty);
        return Event::Wipeout;
    }

    // Best spin (largest rotation reached) plus the grab, if held long enough.
    int best_spin = 0;
    float best_rot = -1.f;
    int grab_pts = 0;
    for (int i = 0; i < TRICK_COUNT; ++i) {
        const TrickRow& t = TRICKS[i];
        if (!trick_matches(t, r)) continue;
        if (t.kind == TrickKind::Grab) {
            grab_pts = std::max(grab_pts, t.points);
        } else if (t.rotation_deg > best_rot) {
            best_rot = t.rotation_deg;
            best_spin = t.points;
        }
    }
    const int base = best_spin + grab_pts;
    if (base == 0) return Event::Landed;

    r.combo = std::min(r.combo + 1, SCORING.combo_max);
    const float mult = 1.f + SCORING.combo_step * static_cast<float>(r.combo - 1);
    r.last_trick_points = static_cast<int>(static_cast<float>(base) * mult);
    r.score += r.last_trick_points;
    return Event::Trick;
}

inline Event step(Rider& r, const Input& in, const Ground& g, float dt) {
    const BoardRow& b = BOARD;
    const float G = WORLD.gravity_ms2;
    constexpr float kDegToRad = 3.14159265f / 180.f;

    if (r.grounded) {
        r.z_m = g.height_m;
        const bool on_snow = g.surface >= 0 && g.surface < SURFACE_COUNT;
        const float fric_mult = on_snow ? SURFACES[g.surface].friction_mult : 1.f;
        const float steer_mult = on_snow ? SURFACES[g.surface].steer_mult : 1.f;
        if (on_snow && g.slope_deg >= b.min_slope_deg) {
            const float rad = g.slope_deg * kDegToRad;
            r.speed += (G * b.gravity_scale * std::sin(rad) - b.friction_snow * fric_mult * G * std::cos(rad)) * dt;
        } else {
            r.speed -= b.friction_other * r.speed * dt;
        }
        if (in.brake) r.speed -= b.brake_drag * r.speed * dt;
        r.speed = std::clamp(r.speed, 0.f, b.max_speed);

        const float turn = b.steer_rate_deg_s * b.carve_grip * steer_mult * std::min(1.f, r.speed / b.turn_speed_ref);
        r.heading_deg += in.steer * turn * dt;

        if (in.jump) {
            r.vz = b.jump_impulse;
            r.grounded = false;
            r.air_time = 0.f;
            r.spin_deg = 0.f;
            r.grab_time = 0.f;
            return Event::Jumped;
        }
        return Event::None;
    }

    r.vz -= G * dt;
    r.z_m += r.vz * dt;
    r.air_time += dt;
    const float spin = (in.spin_right ? 1.f : 0.f) - (in.spin_left ? 1.f : 0.f);
    r.spin_deg += spin * b.air_spin_rate_deg_s * b.air_control * dt;
    if (in.grab) r.grab_time += dt;
    if (r.z_m <= g.height_m && r.vz <= 0.f) return land(r, g);
    return Event::None;
}

}  // namespace snow

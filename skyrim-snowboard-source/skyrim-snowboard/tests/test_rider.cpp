#include <cstdio>
#include <cstdlib>

#include "rider.h"

using namespace snow;

static int failures = 0;
#define CHECK(cond)                                                     \
    do {                                                                \
        if (!(cond)) {                                                  \
            std::printf("FAIL line %d: %s\n", __LINE__, #cond);         \
            ++failures;                                                 \
        }                                                               \
    } while (0)

static const float DT = 1.f / 60.f;

// Run one jump: hold the given inputs for hold_s seconds, then release; return the landing event.
static Event do_jump(Rider& r, bool spin_l, bool spin_r, bool grab, float hold_s, float grab_s = 0.f) {
    Ground flat{0.f, 0.f, 0};
    Input j;
    j.jump = true;
    step(r, j, flat, DT);
    float t = 0.f;
    for (int i = 0; i < 600; ++i) {
        Input in;
        in.spin_left = spin_l && t < hold_s;
        in.spin_right = spin_r && t < hold_s;
        in.grab = grab && t < grab_s;
        Event e = step(r, in, flat, DT);
        t += DT;
        if (e == Event::Trick || e == Event::Wipeout || e == Event::Landed) return e;
    }
    return Event::None;
}

int main() {
    Ground slope{12.f, 12.f, 0};
    Ground flat{0.f, 0.f, 0};
    Input none;

    // Surfaces: ice is faster than packed snow, powder is slower. Heading turns less on ice.
    {
        Rider packed, powder, ice;
        Ground gp{0.f, 12.f, static_cast<int>(SurfaceId::packed)};
        Ground gw{0.f, 12.f, static_cast<int>(SurfaceId::powder)};
        Ground gi{0.f, 12.f, static_cast<int>(SurfaceId::ice)};
        for (int i = 0; i < 60 * 3; ++i) {
            step(packed, none, gp, DT);
            step(powder, none, gw, DT);
            step(ice, none, gi, DT);
        }
        CHECK(ice.speed > packed.speed);
        CHECK(packed.speed > powder.speed);
        Rider a, b;
        a.speed = b.speed = 12.f;
        Input rt;
        rt.steer = 1.f;
        for (int i = 0; i < 30; ++i) {
            step(a, rt, gp, DT);
            step(b, rt, gi, DT);
        }
        CHECK(a.heading_deg > b.heading_deg);
    }

    // Downhill on snow gains speed, capped at max_speed.
    Rider r;
    for (int i = 0; i < 60; ++i) step(r, none, slope, DT);
    CHECK(r.speed > 1.0f);
    for (int i = 0; i < 60 * 120; ++i) step(r, none, slope, DT);
    CHECK(r.speed <= BOARD.max_speed + 1e-3f);
    CHECK(r.speed > BOARD.max_speed - 1.0f);

    // Flat ground slows the rider down.
    Rider f;
    f.speed = 10.f;
    for (int i = 0; i < 60 * 5; ++i) step(f, none, flat, DT);
    CHECK(f.speed < 1.0f);

    // Non-snow slope also slows the rider.
    Rider rock;
    rock.speed = 10.f;
    Ground rocky{12.f, 12.f, -1};
    for (int i = 0; i < 60 * 5; ++i) step(rock, none, rocky, DT);
    CHECK(rock.speed < 1.0f);

    // Braking slows more than coasting.
    Rider a, b;
    a.speed = b.speed = 15.f;
    Input brake;
    brake.brake = true;
    for (int i = 0; i < 60; ++i) {
        step(a, none, slope, DT);
        step(b, brake, slope, DT);
    }
    CHECK(b.speed < a.speed);

    // Steering turns the board only while moving.
    Rider still, moving;
    moving.speed = 12.f;
    Input right;
    right.steer = 1.f;
    for (int i = 0; i < 30; ++i) {
        step(still, right, flat, DT);
        step(moving, right, flat, DT);
    }
    CHECK(moving.heading_deg > 20.f);
    CHECK(still.heading_deg < 1.f);

    // Clean frontside 180 scores 100.
    Rider t1;
    t1.speed = 10.f;
    CHECK(do_jump(t1, true, false, false, 0.55f) == Event::Trick);
    CHECK(t1.score == 100);
    CHECK(t1.combo == 1);

    // Second trick in a row gets the combo bonus (100 * 1.25).
    CHECK(do_jump(t1, false, true, false, 0.55f) == Event::Trick);
    CHECK(t1.last_trick_points == 125);

    // Half-finished spin (about 90 degrees) is a wipeout and resets combo.
    CHECK(do_jump(t1, true, false, false, 0.25f) == Event::Wipeout);
    CHECK(t1.combo == 0);
    CHECK(t1.score == 25);  // 100 + 125 earned, minus the 200 wipeout penalty

    // Grab alone scores the grab trick.
    Rider g1;
    g1.speed = 10.f;
    CHECK(do_jump(g1, false, false, true, 0.f, 0.8f) == Event::Trick);
    CHECK(g1.score == 150);

    // Spin plus grab adds both.
    Rider sg;
    sg.speed = 10.f;
    CHECK(do_jump(sg, true, false, true, 0.55f, 0.8f) == Event::Trick);
    CHECK(sg.score == 250);

    // Plain hop (no spin, no grab) lands without a trick.
    Rider hop;
    hop.speed = 10.f;
    CHECK(do_jump(hop, false, false, false, 0.f) == Event::Landed);
    CHECK(hop.score == 0);

    // Wipeout costs speed.
    Rider w;
    w.speed = 20.f;
    CHECK(do_jump(w, true, false, false, 0.25f) == Event::Wipeout);
    CHECK(w.speed < 8.f);

    if (failures == 0) std::printf("all rider tests passed\n");
    return failures == 0 ? 0 : 1;
}

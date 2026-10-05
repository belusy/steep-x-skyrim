#include <cstdio>
#include <vector>

#include "controller.h"

using namespace snow;

static int failures = 0;
#define CHECK(cond)                                             \
    do {                                                        \
        if (!(cond)) {                                          \
            std::printf("FAIL line %d: %s\n", __LINE__, #cond); \
            ++failures;                                         \
        }                                                       \
    } while (0)

struct FakeGame : IGame {
    bool equipped = false;
    bool ground_known = true;
    Input input;
    Ground ground{0.f, 12.f, 0};
    int apply_riding = 0, apply_released = 0;
    std::vector<Event> events;

    bool board_equipped() override { return equipped; }
    Input read_input() override { return input; }
    bool read_ground(Ground& out) override {
        out = ground;
        return ground_known;
    }
    void apply(const Rider&, bool riding) override { riding ? ++apply_riding : ++apply_released; }
    void notify(Event e, const Rider&) override { events.push_back(e); }
};

int main() {
    const float dt = 1.f / 60.f;
    FakeGame g;
    Controller c(g);

    // Board stowed: nothing happens, the game keeps control.
    for (int i = 0; i < 10; ++i) c.update(dt);
    CHECK(!c.riding());
    CHECK(g.apply_riding == 0);

    // Equip: starts riding and gains speed downhill.
    g.equipped = true;
    for (int i = 0; i < 120; ++i) c.update(dt);
    CHECK(c.riding());
    CHECK(c.rider().speed > 1.f);
    CHECK(g.apply_riding == 120);

    // Unknown ground: no state change, no apply.
    g.ground_known = false;
    const float before = c.rider().speed;
    const int applied = g.apply_riding;
    c.update(dt);
    CHECK(c.rider().speed == before);
    CHECK(g.apply_riding == applied);
    g.ground_known = true;

    // Jump produces a Jumped event.
    g.input.jump = true;
    c.update(dt);
    g.input.jump = false;
    CHECK(!g.events.empty() && g.events.back() == Event::Jumped);

    // Stow: control handed back exactly once.
    g.equipped = false;
    c.update(dt);
    c.update(dt);
    CHECK(!c.riding());
    CHECK(g.apply_released == 1);

    if (failures == 0) std::printf("all controller tests passed\n");
    return failures == 0 ? 0 : 1;
}

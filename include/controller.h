// Glue between the rider logic and the game. The game is reached only through IGame,
// so this file is unit-tested on Linux with a fake game (tests/test_controller.cpp).
// The real Skyrim implementation of IGame lives in plugin/ and is NOT yet written/verified.
#pragma once
#include "rider.h"

namespace snow {

struct IGame {
    virtual ~IGame() = default;
    virtual bool board_equipped() = 0;
    virtual Input read_input() = 0;
    // Fill terrain height, slope and surface under the rider. Return false if unknown this frame.
    virtual bool read_ground(Ground& out) = 0;
    // Push the rider's state to the game (movement, heading, board pose).
    virtual void apply(const Rider& r, bool riding) = 0;
    virtual void notify(Event e, const Rider& r) = 0;
};

class Controller {
public:
    explicit Controller(IGame& game) : game_(game) {}

    void update(float dt) {
        if (!game_.board_equipped()) {
            if (riding_) {
                riding_ = false;
                game_.apply(rider_, false);  // hand movement back to the game
            }
            return;
        }
        Ground ground;
        if (!game_.read_ground(ground)) return;  // keep last state, try again next frame
        if (!riding_) {
            riding_ = true;
            const int keep_score = rider_.score;
            rider_ = Rider{};
            rider_.score = keep_score;
            rider_.z_m = ground.height_m;
        }
        const Input in = game_.read_input();
        const Event e = step(rider_, in, ground, dt);
        game_.apply(rider_, true);
        if (e != Event::None) game_.notify(e, rider_);
    }

    bool riding() const { return riding_; }
    const Rider& rider() const { return rider_; }

private:
    IGame& game_;
    Rider rider_;
    bool riding_ = false;
};

}  // namespace snow

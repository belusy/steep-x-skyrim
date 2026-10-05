.PHONY: sheets test plugin

sheets:
	python3 tools/preflight.py --scope logic
	python3 tools/gen.py

test: sheets
	g++ -std=c++17 -Wall -Wextra -Iinclude tests/test_rider.cpp -o build_test_rider
	./build_test_rider
	g++ -std=c++17 -Wall -Wextra -Iinclude tests/test_controller.cpp -o build_test_controller
	./build_test_controller
	rm -f build_test_rider build_test_controller

# The real Skyrim plugin is blocked until the FULL preflight is clean
# (hooks need verified addresses for the player's game version, runs need real coordinates).
plugin:
	python3 tools/preflight.py --scope all
	@echo "Preflight clean. Plugin build is not wired up yet: plugin/ is still to be written."

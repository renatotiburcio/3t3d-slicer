#include <catch2/catch_test_macros.hpp>

#include "slic3r/Utils/AnycubicWorkbenchState.hpp"

using namespace Slic3r;

TEST_CASE("Anycubic Kobra 4 Combo simulator starts with a safe disconnected snapshot", "[Anycubic][Workbench]")
{
    AnycubicKobra4Simulator simulator;
    const AnycubicWorkbenchSnapshot& snapshot = simulator.snapshot();

    CHECK(snapshot.model == "Anycubic Kobra 4 Combo");
    CHECK(snapshot.display_name == "Anycubic Kobra 4 Combo");
    CHECK(snapshot.state == AnycubicWorkbenchState::disconnected);
    CHECK(snapshot.host == "simulator://anycubic-kobra-4-combo");
    CHECK(snapshot.camera_available);
    CHECK(snapshot.ace_available);
    CHECK(snapshot.ace_slot_count == 4);
    CHECK(snapshot.progress_percent == 0.0);
}

TEST_CASE("Anycubic Kobra 4 Combo simulator advances printing by layers", "[Anycubic][Workbench]")
{
    AnycubicKobra4Simulator simulator;
    simulator.set_progress(1, 4);
    simulator.set_state(AnycubicWorkbenchState::printing);
    simulator.tick(120);

    CHECK(simulator.snapshot().current_layer == 3);
    CHECK(simulator.snapshot().total_layers == 4);
    CHECK(simulator.snapshot().progress_percent == 75.0);
    CHECK(simulator.snapshot().state == AnycubicWorkbenchState::printing);

    simulator.tick(60);
    CHECK(simulator.snapshot().current_layer == 4);
    CHECK(simulator.snapshot().progress_percent == 100.0);
    CHECK(simulator.snapshot().state == AnycubicWorkbenchState::completed);
}

TEST_CASE("Anycubic Kobra 4 Combo simulator does not advance non-printing states", "[Anycubic][Workbench]")
{
    AnycubicKobra4Simulator simulator;
    simulator.set_progress(2, 10);
    simulator.set_state(AnycubicWorkbenchState::paused);
    simulator.tick(600);

    CHECK(simulator.snapshot().current_layer == 2);
    CHECK(simulator.snapshot().progress_percent == 20.0);
    CHECK(simulator.snapshot().state == AnycubicWorkbenchState::paused);
}

TEST_CASE("Anycubic Kobra 4 Combo simulator records skipped objects once", "[Anycubic][Workbench]")
{
    AnycubicKobra4Simulator simulator;
    simulator.skip_objects({0, 2, 2, 99});

    REQUIRE(simulator.snapshot().skipped_object_ids.size() == 2);
    CHECK(simulator.snapshot().skipped_object_ids[0] == 0);
    CHECK(simulator.snapshot().skipped_object_ids[1] == 2);
}

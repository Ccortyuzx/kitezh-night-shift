
#include "kitezh/shift.hpp"
#include "kitezh/vessel.hpp"
#include "kitezh/lock.hpp"

#include <vector>
#include <string>

namespace kitezh {

ShiftReport runNightShift(const std::vector<Vessel>& queue) {
    ShiftReport report;
    std::vector<Vessel> wait;
    std::vector<std::string> seen;
    for (const auto& vessel : queue) {
        bool dubl = false;
        for (const auto& call_sign : seen) {
            if (call_sign == vessel.call_sign) {
                dubl = true;
                break;
            }
        }
        if (dubl) {
            report.refused.push_back(Refusal{vessel.call_sign, "duplicate"});
            continue;
        }
        seen.push_back(vessel.call_sign);
        if (validateVessel(vessel).has_value()) {
            report.refused.push_back(Refusal{vessel.call_sign, "invalid"});
            continue;
        }
        if (vessel.draft_dm > 28) {
            report.refused.push_back(Refusal{vessel.call_sign, "draft"});
            continue;
        }
        wait.push_back(vessel);
    }
    Lock lock;
    while (!wait.empty()) {
        std::vector<Vessel> part;
        Vessel one = wait[0];
        part.push_back(one);
        wait.erase(wait.begin());
        if (one.kind != VesselKind::Oversized) {
            Direction direction = one.direction;
            bool skip = false;
            std::size_t i = 0;
            while (i < wait.size()) {
                const Vessel& vessel = wait[i];
                bool can_take = vessel.direction == direction && vessel.kind != VesselKind::Oversized && 
                (!skip || vessel.kind == VesselKind::Ambulance) &&
                    fitsInChamber(part, vessel);
                if (can_take) {
                    part.push_back(vessel);
                    wait.erase(wait.begin() + i);
                } else {
                    skip = true;
                    ++i;
                }
            }
        }
        bool upstream = one.direction == Direction::Upstream;
        Level entry_level = upstream ? Level::Low : Level::High;
        Gate entry_gate = upstream ? Gate::Lower : Gate::Upper;
        Gate exit_gate = upstream ? Gate::Upper : Gate::Lower;
        if (lock.level() != entry_level) {
            lock.pump();
        }
        lock.open(entry_gate);
        for (const auto& vessel : part) {
            lock.enter(vessel);
        }
        lock.close(entry_gate);
        lock.pump();
        lock.open(exit_gate);
        for (const auto& vessel : part) {
            lock.exit(vessel.call_sign);
            report.passed.push_back(vessel.call_sign);
        }
        lock.close(exit_gate);
    }
    report.pump_cycles = lock.pumpCycles();
    return report;
}

}  // namespace kitezh

#include "kitezh/lock.hpp"
#include "kitezh/vessel.hpp"
#include <stdexcept>

namespace kitezh {

Lock::Lock() {
    level1 = Level::Low;
    low_gate = GateState::Closed;
    high_gate = GateState::Closed;
    pump_cycle = 0;
}

Level Lock::level() const {
    return level1;
}

GateState Lock::gate(Gate gate_1) const {
    if (gate_1 == Gate::Lower){
        return low_gate;
    }
    if (gate_1 == Gate::Upper){
        return high_gate;
    }
    throw LockError(" ");
    
}

int Lock::pumpCycles() const {
    return pump_cycle;
}

std::vector<Vessel> Lock::inside() const {
    return vessels;
}

void Lock::open(Gate gate_1) {
    if (gate_1 == Gate::Lower){
        if(level1 != Level::Low){
            throw LockError("Затвор не на нижнем уровне");
        }
        if (high_gate == GateState::Open){
            throw LockError("Оба сразу открыты быть не могут.");
        }
        low_gate = GateState::Open;
    }else{
        if (level1 != Level::High){
            throw LockError("Затвор не на верхнем уровне");
        }
        if (low_gate == GateState::Open){
            throw LockError("Оба сразу открыты быть не могут.");
        }
        high_gate = GateState::Open;
    }

    
}

void Lock::close(Gate gate_1) {
    if (gate_1 == Gate::Lower){
        low_gate = GateState::Closed;
    }else{
        high_gate = GateState::Closed;
    }
}

void Lock::pump() {
    if (low_gate == GateState::Open || high_gate == GateState::Open){
        throw LockError("Насос работает только когда оба закрыты.");
    }
    Level nextLevel;
    int depth;
    if (level1 == Level::Low){
        nextLevel = Level::High;
        depth = kHighDepthDm;
    }else{
        nextLevel = Level::Low;
        depth = kLowDepthDm;
    }
    for (size_t i = 0; i != vessels.size(); ++i){
        if (vessels[i].draft_dm > depth){
            throw LockError("Недостаточная глубина");
        }
    }
    level1 = nextLevel;
    pump_cycle += 1;
}


void Lock::enter(const Vessel& vessel) {
    if (validateVessel(vessel).has_value()){
        throw LockError("Brrrboom");
    }
    for (size_t i = 0; i < vessels.size(); ++i){
        if (vessels[i].call_sign == vessel.call_sign){
            throw LockError("Такое позывное было");
        }
    }
    int now_depth = (level1 == Level::Low ? kLowDepthDm : kHighDepthDm);
    if (vessel.draft_dm > now_depth){
        throw LockError("Нижний затвор закрыто");
    }
    if (vessel.direction == Direction::Upstream){
        if (level1 == Level::High || low_gate == GateState::Closed){
            throw LockError("Нижний затвор закрыт");
        }
    }else{
        if (level1 == Level::Low || high_gate == GateState::Closed){
            throw LockError("Верхний затвор закрыт");
        }
    }
    if (vessel.kind == VesselKind::Oversized && !vessels.empty()){
        throw LockError("Негабарит");
    }
    if (!vessels.empty() && vessels[0].kind == VesselKind::Oversized){
        throw LockError("Нельзя к негабариту");
    }
    if (!fitsInChamber(vessels, vessel)){
        throw LockError("Судно не помещается по длине");
    }
    vessels.push_back(vessel);

}

void Lock::exit(const std::string& call_sign) {
    if (vessels.empty()){
        throw LockError("Камера пустая");
    }
    if (vessels[0].call_sign != call_sign){
        throw LockError("Судно не первое");
    }
    if (vessels[0].direction == Direction::Upstream){
        if (level1 == Level::Low || high_gate == GateState::Closed){
            throw LockError("Lfoh");
        }
    }else{
        if (level1 == Level::High || low_gate == GateState::Closed){
            throw LockError("Верхний затвор закрыт");
        }
    }
    vessels.erase(vessels.begin());
}

}  // namespace kitezh

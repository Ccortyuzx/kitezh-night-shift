#include "kitezh/vessel.hpp"
#include <stdexcept>
#include <optional>
#include <string>
namespace kitezh {

std::optional<std::string>validateVessel(const Vessel& Vessel) {
    if (Vessel.call_sign.empty()){
        return "empty call sign";
    }
    for (char el : Vessel.call_sign){
        if (el == ' ' || el == '\t' || el == '\n'){
            return "bad call sign";
        }
    }
    if (Vessel.length_m <= 0 || Vessel.length_m > 36){
        return "bad length";
    }
    if (Vessel.draft_dm <= 0){
        return "bad draft";
    }
    return std::nullopt;
}

bool fitsInChamber(const std::vector<Vessel>& inside, const Vessel& next) {
    double sum_total = next.length_m;
    for (auto& el : inside){
        sum_total += el.length_m;
    }
    int count = inside.size() + 1;
    if (count > 1){
        sum_total += (count - 1) * kGapM;
    }
    return sum_total <= kChamberLengthM;
}

}  // namespace kitezh

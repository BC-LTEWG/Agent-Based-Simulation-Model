#include "Society.h"
#include <numeric>

bool Society::meetsStandardForLowerWorkingHours() {
  double sum = 0.0;
  for(auto& firm : firms) {
    sum += firm.getAvgProductivity();
  }
  double avgProductivity = sum / firms.size();
  return avgProductivity >= productivityThreshold;
}
#include "Machine.h"
#include "Firm.h"
#include <string>
#include <vector>
#include <unordered_map>

std::vector<Firm&> Machine::addMachinetoFirm(Firm& f, std::vector<Machine&> machines) {
  f.machines.push_back(machines);
  return f.machines;
}

Machine& machine : getHoursUsed() {
  return this->hoursUsed;
}

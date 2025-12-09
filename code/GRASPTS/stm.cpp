#ifndef STM_CPP
#define STM_CPP

#include <map>

struct STM {
  std::map<int, int> stm;  // (seq, id) -> seq
  int tau;
  int seq = 0;

  STM(int tau) : tau(tau) {}

  void MarkTabu(int id) {
    seq++;
    stm[id] = seq;
  }

  bool isTabu(int id, int solution_elements_size) {
    // Um elemento é tabu se estiver na STM e tau*solution_elements_size (quantidade de elementos permitidoas na STM) não foi ultrapassado
    const int max_tabu_size = static_cast<int>(tau * solution_elements_size);
    const auto position = stm[id];
    return position != 0 && (seq - position) <= max_tabu_size;
  }
};

#endif 
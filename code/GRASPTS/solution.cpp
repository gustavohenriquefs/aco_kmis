#ifndef SOLUCAO_CPP
#define SOLUCAO_CPP

#include <set>
#include <vector>

#include "../bibliotecas/roaring.hh"

using Subset = roaring::Roaring;

// Classe para gerenciar a Solução (S ou Sb)
class Solucao {
 private:
  std::set<int> solution_ids;
  roaring::Roaring solution;
  std::vector<Subset> F;  // connections for ACO

  uint64_t intersection_cardinality = 0;

  void calc_solution() {
    int i = 0;
    for (int e : this->solution_ids) {
      if (i) {
        this->solution &= F[e];
      } else {
        this->solution = F[e];
      }

      i++;
    }
    this->intersection_cardinality = this->solution.cardinality();
  }

 public:
  Solucao() {}

  Solucao(std::vector<Subset>& F) : F(F) {}

  Subset get_solution() const {
    return this->solution;
  }

  Subset calculate_B_prime(int removed_e) const {
    Subset B_prime;
    bool first = true;

    // Apenas a interseção dos elementos restantes é calculada
    for (int e : this->solution_ids) {
      if (removed_e != e) {
        if (first) {
          B_prime = this->F[e];  // Assume que F é acessível pelo índice e
          first = false;
        } else {
          B_prime = B_prime & this->F[e];
        }
      }
    }
    return B_prime;
  }

  void set_solucao(const Solucao S) {
    this->solution_ids = S.solution_ids;
    this->calc_solution();
  }

  std::set<int> get_indices() const {
    return solution_ids;
  }

  uint64_t get_valor() const {
    return intersection_cardinality;
  }

  Subset get_intersection() const {
    return this->solution;
  }

  bool has_element(int e) const {
    return this->solution_ids.find(e) != this->solution_ids.end();
  }

  void swap(int ei, int ej) {
    this->solution_ids.erase(ei);
    this->solution_ids.insert(ej);
    this->calc_solution();
  }

  void add_item_idx(int idx) {
    if (solution_ids.empty()) {
      solution_ids.insert(solution_ids.end(),
                          idx);
      solution = this->F[idx];
      intersection_cardinality = solution.cardinality();
      return;
    }

    this->solution_ids.insert(this->solution_ids.end(), idx);
    this->solution = this->solution & this->F[idx];
    intersection_cardinality = solution.cardinality();
  }

  bool operator>(const Solucao& outra) const {
    return this->get_valor() > outra.get_valor();
  }
};

#endif  // SOLUCAO_CPP
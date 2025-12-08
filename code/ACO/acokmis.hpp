#pragma once
#include <set>
#include <vector>

#include "../Report/report.cpp"
#include "../bibliotecas/roaring.hh"

class ACO {
 protected:
  std::vector<roaring::Roaring> connections;
  double alpha_;
  double beta_;
  double tau_0_;
  double rho_;
  int iter_max_;
  int numUsers;

  std::vector<std::vector<double>>
      pheromone_matrix_;

 public:

//  Os parâmetros utilizados no algoritmo ACO KMIS foram obtidos
// também de forma empírica e são eles: α = 0.5, β = 2, τ0 = 1, ρ = 0.7 e IterMax = 50.
// Vale destacar que, pelo fato dos algoritmos GRASP REATIVO e VNS REATIVO possuírem
// componentes de aleatoriedade, estes algoritmos foram executados 10 vezes por instância, e a
// solução e o tempo de execução considerados, foram obtidos através da média das 10 execuções.
  ACO(std::vector<roaring::Roaring> connections,
      int numUsers,
      int numIterations,
      double alpha = 0.5,
      double beta = 2.0,
      double tau_0 = 1.0,
      double rho = 0.7,
      int iter_max = 50)
      : connections(connections),
        numUsers(numUsers),
        alpha_(alpha),
        beta_(beta),
        tau_0_(tau_0),
        rho_(rho),
        iter_max_(iter_max) {}

  virtual ~ACO() = default;

  virtual std::vector<ReportExecData> solve_kMIS(int k) = 0;
};

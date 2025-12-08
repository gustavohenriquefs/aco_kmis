#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <random>
#include <set>
#include <unordered_map>
#include <vector>

#include "../Report/report-manager.cpp"
#include "../bibliotecas/roaring.hh"
#include "./instance_i.cpp"

#define get_current_time() std::chrono::high_resolution_clock::now()
#define TIME_DIFF(start, end) std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()

#define sz(v) ((int)v.size())

using Subset = roaring::Roaring;

struct STM {
  std::unordered_map<int, int> map;

  void MarkTabu(int id, int tenure, int currentIter) {
    map[id] = currentIter + tenure;
  }

  bool isTabu(int id, int currentIter) {
    auto it = map.find(id);

    if (it != map.end()) {
      return currentIter < it->second;
    }

    return false;
  }
};

// Classe para gerenciar a Solução (S ou Sb)
class Solucao {
 private:
  std::set<int> solution_ids;
  roaring::Roaring solution;
  std::vector<Subset> F;  // connections for ACO

  int intersection_cardinality = 0;

  void unsetBit(int e) {
    if (this->solution.getIndex(e) & 0) {
      std::cerr << "[faild]: try remove a unset bit" << std::endl;
      return;
    }

    this->solution.removeChecked(e);
    this->solution_ids.erase(solution_ids.find(e));
  }

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

  Subset get_solution() {
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

  int get_valor() const {
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

// Classe Principal GRASPTs
class GRASPTs {
 private:
  // Parâmetros da Meta-heurística
  InstanceI I;                   // Instância (E, F, k)
  int IterMax;                   // Delta (Δ) no pseudocódigo (Número de iterações GRASP)
  double alphaRG;                // αRG para CRG (e.g., 0.50, a variante mais eficiente)
  int tenure_tau;                // τ para Busca Tabu (e.g., 0.5 vezes |L| ou constante)
  int maxIterSemMelhoria_gamma;  // γ para Busca Tabu

  Solucao melhorSolucaoGlobal;  // Sb (Best solution found)
  std::mt19937 rng;             // Gerador de números aleatórios

  /**
   * Auxiliar: Calcula o valor guloso g(c) para o candidato c.
   * g(c) é o número de features que os elementos em S parcial têm em comum com c.
   */
  int funcaoGuloso(Subset& S_parcial_indices, int indice_candidato) {
    return (S_parcial_indices & I.featuresF[indice_candidato]).cardinality();
  }

  // ====================================================================
  // FASE 1: CONSTRUÇÃO (Construct)
  // Implementa Constructive Random-Greedy (CRG) - Algoritmo 3
  // ====================================================================
  Solucao construir_CRG(double alphaRG) {
    // Mapeia os passos 1-10 do Algoritmo 3
    Solucao S(I.featuresF);  // Passo 1: S ← ∅

    std::uniform_int_distribution<int> dist(0, I.indicesE.size() - 1);
    int ie = dist(rng);

    S.add_item_idx(ie);

    std::vector<int> CL;

    for (int e : I.indicesE)
      if (e != ie) {
        CL.push_back(e);
      }

    while (S.get_indices().size() < static_cast<size_t>(I.k)) {
      // Passo 5: RCL ← SelectRandom(CL, αRG · |CL|)
      std::vector<int> RCL = select_random(CL, alphaRG * sz(CL));
      // Passo 6: e ← argmaxc∈RCL g(c)
      std::pair<int, int> e = {RCL[0], funcaoGuloso(S.get_solution(), RCL[0])};

      for (int c = 1; c < sz(RCL); ++c) {
        int g_c = funcaoGuloso(S.get_solution(), RCL[c]);

        if (e.second < g_c) {
          e = {c, g_c};
        }
      }

      S.add_item_idx(RCL[e.first]);
      for (int i = 0; i < sz(CL); i++)
        if (CL[i] == RCL[e.first]) {
          CL.erase(CL.begin() + i);
        }
    }

    return S;
  }

  std::vector<int> select_random(std::vector<int>& CL, int num_elements) {
    std::vector<int> RCL;
    std::sample(CL.begin(), CL.end(),
                std::back_inserter(RCL),
                num_elements,
                rng);
    return RCL;
  }

  // ====================================================================
  // FASE 2: MELHORIA (Improve)
  // Implementa Tabu Search (TS) - Algoritmo 5
  // ====================================================================
  Solucao busca_tabu(Solucao S, int tau, int gamma, std::vector<ReportExecData>& reports) {
    // Mapeia os passos 1-30 do Algoritmo 5
    Solucao S_atual = S;
    Solucao Sb = S;  // Sb ← S (passo 1)

    STM STM;        // Memória de Curto Prazo (passo 2)
    int delta = 0;  // Iterações sem melhoria (γ, passo 3)

    int it = 0;
    do {
      it++;

      bool improve = false;  // Passo 4: Improve ← false

      std::tuple<int, int, int> best_move = {-1, -1, -1};
      // Passo 7: for ei ∈ S \ STM do
      for (int ei : S.get_indices())
        if (!STM.isTabu(ei, it)) {
          Subset B_1 = S.calculate_B_prime(ei);

          // Passo 8: for ej ∈ E \ S do
          for (int ej : I.indicesE)
            if (!S.has_element(ej)) {
              // Passo 9: ΔkMIS ← kMIS((S \ ei) U ej) - kMIS(S)
              Subset B_2 = B_1 & I.featuresF[ej];

              if (B_2.cardinality() > Sb.get_valor()) {
                S.swap(ei, ej);

                improve = true;

                delta = 0;
                Sb = S;
                STM.MarkTabu(ej, tenure_tau, it);

                break;
              }

              if (!improve && B_2.cardinality() > std::get<2>(best_move)) {
                assert(("[failed]: best_move has a invalid value (1)", std::get<0>(best_move) >= 0));
                best_move = {ei, ej, B_2.cardinality()};
              }
            }

          if (improve) break;
        }

      // Passo 10: if ΔkMIS > BestDelta then
      if (!improve) {
        assert(("[failed]: best_move has a invalid value (2)", std::get<0>(best_move) >= 0));
        S.swap(std::get<0>(best_move), std::get<1>(best_move));
        delta++;
        STM.MarkTabu(std::get<1>(best_move), tenure_tau, it);
      }
    } while (delta < gamma);  // Passo 29: until $\Delta = \gamma$


    return Sb;
  }

 public:
  GRASPTs(const InstanceI& instance, int maxIt, double alpha, int tau, int gamma)
      : I(instance),
        IterMax(maxIt),
        alphaRG(alpha),
        tenure_tau(tau),
        maxIterSemMelhoria_gamma(gamma),
        rng(std::random_device{}()),
        melhorSolucaoGlobal(I.featuresF) {
  }

  GRASPTs(const InstanceI& instance) : I(instance) {
    IterMax = 1000;
    alphaRG = 0.50;
    tenure_tau = 0.5;
    maxIterSemMelhoria_gamma = 5;
    rng = std::mt19937(std::random_device{}());
    melhorSolucaoGlobal = Solucao(I.featuresF);
  }

  /**
   * Método principal para executar o algoritmo GRASPTs.
   * Mapeia o Algoritmo 1: GRASP.
   */
  std::vector<ReportExecData> resolver() {
    vector<ReportExecData> reports;
    auto start_time = get_current_time();

    // Sb ← ∅ (passo 1, inicializado no construtor)
    for (int i = 0; i < this->IterMax; ++i) {  // for i ∈ 1 . . . Δ do (passo 2)
      // 3: S ← Construct(I, α)
      Solucao S_construida = construir_CRG(alphaRG);

      if (S_construida > melhorSolucaoGlobal) {
        reports.push_back(ReportExecData(S_construida.get_indices(), TIME_DIFF(start_time, get_current_time())));
      }

      // 4: S' ← Improve(S)
      Solucao S_melhorada = busca_tabu(S_construida, tenure_tau, maxIterSemMelhoria_gamma, reports);

      // 5: if kMIS(S') > kMIS(Sb) then 6: Sb ← S'
      if (S_melhorada > melhorSolucaoGlobal) {
        melhorSolucaoGlobal.set_solucao(S_melhorada);

        reports.push_back(ReportExecData(S_melhorada.get_indices(), TIME_DIFF(start_time, get_current_time())));
      }
    }
    return reports;  // 9: return Sb
  }
};
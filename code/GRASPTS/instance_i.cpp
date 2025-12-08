#ifndef INSTANCEI
#define INSTANCEI

#include <vector>

#include "../bibliotecas/roaring.hh"

using Subset = roaring::Roaring;

// Estrutura I: Representa a instância do problema kMIS (E, F, k)
struct InstanceI {
  int k;                          // Número de elementos a serem selecionados
  std::vector<int> indicesE;      // Conjunto de índices dos elementos E
  std::vector<Subset> featuresF;  // Conjunto F de features (indexado pelos índices de E)
};

#endif
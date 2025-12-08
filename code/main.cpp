#include <iostream>

#include "./GRASPTS/instance_i.cpp"
#include "./Report/report-manager.cpp"
#include "ACO/acokmis.cpp"
#include "GRASPTS/graspts.cpp"
#include "Intances/instances.cpp"

#define get_current_time() std::chrono::high_resolution_clock::now()
#define TIME_DIFF(start, end) std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()

// Function to process ACO for a given instance
// @param instance The instance to process
void processACO(const Instance& instance, ReportManager& report_manager) {
  ACOKMIS aco_kmis = ACOKMIS(
      instance.get_connections(),
      instance.get_num_elements_l(),
      instance.get_num_elements_r());

  auto exec_reports = aco_kmis.solve_kMIS(instance.get_k());

  Report report_instance(instance.get_connections(),
                         instance.get_file_name(),
                         instance.get_k(),
                         exec_reports);

  report_manager.add_reports(report_instance);
}

InstanceI mapACOInstanceToGRASPTsInstance(const Instance& i) {
  InstanceI ni;

  ni.k = i.get_k();

  ni.featuresF = i.get_connections();

  for (int i = 0; i < sz(ni.featuresF); ++i) {
    ni.indicesE.push_back(i);
  }

  return ni;
}

void processGRASPTs(const Instance& instance, ReportManager& report_manager) {
  InstanceI i = mapACOInstanceToGRASPTsInstance(instance);
  GRASPTs grasp = GRASPTs(i);
  grasp.resolver();
}

// // Function to execute ACO and measure time
// // @param instance The instance to process
// void executeAlgorithmACO(const Instance& instance, ReportManager& report_manager) {
//   const auto start_time = get_current_time();

//   processeACO(instance, report_manager);

//   const auto end_time = get_current_time();
//   const auto duration = TIME_DIFF(start_time, end_time);
// }

int main() {
  std::cout << "-----------------começou-----------------------" << std::endl;
  IntancesReader reader = IntancesReader();
  const auto& instances = reader.get_instances();
  std::cout << "-----------------leu instancia-----------------------" << std::endl;

  ReportManager report_manager = ReportManager("graspts");

  // Execute ACO for each instance
  // for (auto instance : instances) {
  //   std::cout << "check instance: " << instance.get_file_name() << endl;

  //   processACO(instance, report_manager);
  // }

  for (auto instance : instances) {
    std::cout << "check instance: " << instance.get_file_name() << endl;
    for (int i = 0; i < 10; i++)
      processGRASPTs(instance, report_manager);
  }

  std::cout << "-----------------acabou-----------------------" << std::endl;
}
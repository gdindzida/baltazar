#define PROFILELOG

#include "baltazar/dag.hpp"
#include "baltazar/parallel.hpp"
#include "baltazar/thread_pool.hpp"
#include <atomic>
#include <baltazar.hpp>
#include <cstddef>
#include <iostream>
#include <ostream>
#include <random>
#include <filesystem>

namespace fs = std::filesystem;

// Task definition
class SumNValues {
public:
  SumNValues(size_t n) : m_n(n) {}

  int operator()() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 100);

    int ret = 0;
    for (int i = 0; i < m_n; i++) {
      ret += dist(gen);
    }
    std::cout << "Partial sums is: " << ret << std::endl;

    return ret;
  }

private:
  size_t m_n;
};

class Add {
public:
  int operator()(int a, int b) {
    std::cout << "Sum of partial sum is: " << a + b << std::endl;
    return a + b;
  }
};

class Print {
public:
  void operator()(int a) { std::cout << "Result is: " << a << std::endl; }
};

// Identifier definition
enum class TaskIdentifers : size_t { SumId = 0, AddId = 1, PrintId = 2 };

// Application
int main() {
  // Createing a graph
  baltazar::Node<0, SumNValues> sumValuesA{
      SumNValues(250), static_cast<size_t>(TaskIdentifers::SumId)};

  baltazar::Node<0, SumNValues> sumValuesB{
      SumNValues(250), static_cast<size_t>(TaskIdentifers::SumId)};

  baltazar::Node<0, SumNValues> sumValuesC{
      SumNValues(250), static_cast<size_t>(TaskIdentifers::SumId)};

  baltazar::Node<0, SumNValues> sumValuesD{
      SumNValues(250), static_cast<size_t>(TaskIdentifers::SumId)};

  // Sum A _______ Sum [A, B]
  // Sum B ___|
  baltazar::Node<2, Add> addNodeAB{Add(),
                                   static_cast<size_t>(TaskIdentifers::AddId)};
  addNodeAB.setDependencyAt<0>(sumValuesA);
  addNodeAB.setDependencyAt<1>(sumValuesB);

  // Sum C _______ Sum [C, D]
  // Sum D ___|
  baltazar::Node<2, Add> addNodeCD{Add(),
                                   static_cast<size_t>(TaskIdentifers::AddId)};
  addNodeCD.setDependencyAt<0>(sumValuesC);
  addNodeCD.setDependencyAt<1>(sumValuesD);

  // Sum [A, B] _______ Sum [A, B, C, D]
  // Sum [C, D] ___|
  baltazar::Node<2, Add> addNodeABCD{
      Add(), static_cast<size_t>(TaskIdentifers::AddId)};
  addNodeABCD.setDependencyAt<0>(addNodeAB);
  addNodeABCD.setDependencyAt<1>(addNodeCD);

  // Sum [A, B, C, D] _____ Print result
  baltazar::Node<1, Print> printNode{
      Print(), static_cast<size_t>(TaskIdentifers::PrintId)};
  printNode.setDependencyAt<0>(addNodeABCD);

  // Adding nodes randomly to list and sorting them
  baltazar::NodeList<8> nodeList{};

  nodeList.addNode(&sumValuesA);
  nodeList.addNode(&addNodeAB);
  nodeList.addNode(&sumValuesB);
  nodeList.addNode(&printNode);
  nodeList.addNode(&sumValuesC);
  nodeList.addNode(&sumValuesD);
  nodeList.addNode(&addNodeABCD);
  nodeList.addNode(&addNodeCD);

  nodeList.sortNodes();

  // Define profiler
  // Note:
  // Important to define PROFILELOG macro as is in the first line of this file!
  std::string file_path = "temp/output_log.txt";
  fs::path dir_path = fs::path(file_path).parent_path();

  if (!fs::exists(dir_path)) {
      // Create the directory if it does not exist
      if (fs::create_directory(dir_path)) {
          std::cout << "Directory created successfully." << std::endl;
      } else {
          std::cerr << "Failed to create directory." << std::endl;
          return 1; // Exit with an error code
      }
  }

  std::ofstream outfile(file_path);
  if (!outfile) {
    std::cerr << "Error: could not create file\n";
    return 1;
  }

  // Run nodes in parallel once
  baltazar::ParallelCoreRunner<baltazar::core::MultiThreadedCoreProfiler<32>>
      runner{&outfile, true};
  std::atomic<bool> stop{false};
  baltazar::ThreadPool<4, 10> tPool{};
  runner.runNodeListParallelOnce(nodeList, tPool, stop);

  return 0;
}

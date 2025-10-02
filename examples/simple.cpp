#include <atomic>
#include <baltazar.hpp>
#include <cstddef>
#include <iostream>
#include <ostream>
#include <random>

// Task definition
class IntGenerator {
public:
  int operator()() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 100);

    auto ret = dist(gen);
    std::cout << "Generated: " << ret << std::endl;
    return ret;
  }
};

class Add {
public:
  int operator()(int a, int b) {
    std::cout << "Sum is: " << a + b << std::endl;
    return a + b;
  }
};

class Mul {
public:
  int operator()(int a, int b) {
    std::cout << "Product is: " << a * b << std::endl;
    return a * b;
  }
};

class Div {
public:
  double operator()(int a, int b) {
    std::cout << "Result of division is: " << a / b << std::endl;
    return static_cast<double>(a) / static_cast<double>(b);
  }
};

class Print {
public:
  void operator()(double a) { std::cout << "Result is: " << a << std::endl; }
};

// Identifier definition
enum class TaskIdentifers : size_t {
  IntGenId = 0,
  AddId = 1,
  MulId = 2,
  DivId = 3,
  PrintId = 4
};

// Application
int main() {
  // Createing a graph
  baltazar::Node<0, IntGenerator> intGeneratorNodeA{
      IntGenerator(), static_cast<size_t>(TaskIdentifers::IntGenId)};

  baltazar::Node<0, IntGenerator> intGeneratorNodeB{
      IntGenerator(), static_cast<size_t>(TaskIdentifers::IntGenId)};

  baltazar::Node<0, IntGenerator> intGeneratorNodeC{
      IntGenerator(), static_cast<size_t>(TaskIdentifers::IntGenId)};

  // Generate A _______ A + B
  // Generate B ___|
  baltazar::Node<2, Add> addNode{Add(),
                                 static_cast<size_t>(TaskIdentifers::AddId)};
  addNode.setDependencyAt<0>(intGeneratorNodeA);
  addNode.setDependencyAt<1>(intGeneratorNodeB);

  // A + B      _______ (A + B) * C
  // Generate C ___|
  baltazar::Node<2, Mul> mulNode{Mul(),
                                 static_cast<size_t>(TaskIdentifers::MulId)};
  mulNode.setDependencyAt<0>(addNode);
  mulNode.setDependencyAt<1>(intGeneratorNodeC);

  // (A + B) * C _______  A + B
  // Generate C  ___|
  baltazar::Node<2, Div> divNode{Div(),
                                 static_cast<size_t>(TaskIdentifers::DivId)};
  divNode.setDependencyAt<0>(mulNode);
  divNode.setDependencyAt<1>(intGeneratorNodeC);

  // A + B _____ Print result
  baltazar::Node<1, Print> printNode{
      Print(), static_cast<size_t>(TaskIdentifers::PrintId)};
  printNode.setDependencyAt<0>(divNode);

  // Adding nodes randomly to list and sorting them
  baltazar::NodeList<7> nodeList{};

  nodeList.addNode(&divNode);
  nodeList.addNode(&addNode);
  nodeList.addNode(&intGeneratorNodeA);
  nodeList.addNode(&printNode);
  nodeList.addNode(&mulNode);
  nodeList.addNode(&intGeneratorNodeB);
  nodeList.addNode(&intGeneratorNodeC);

  nodeList.sortNodes();

  // Run nodes in sequence once
  baltazar::SerialCoreRunner<> runner{};
  std::atomic<bool> stop{false};
  runner.runNodeListSerialOnce(nodeList, stop);

  return 0;
}

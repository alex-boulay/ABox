#include "ABoxApp.hpp"
#include <exception>
#include <iostream>

int main()
{
  ABoxApp app;
  std::cout << "App created, memory pointer : " << (void *)&app << '\n';
  try {
    app.run();
  }
  catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}


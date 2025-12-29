#include "ABoxApp.hpp"
#include "Logger.hpp"
#include <exception>
#include <iostream>

int main()
{
  ABoxApp app;
  LOG_INFO("App") << "App created, memory pointer: " << (void *)&app;
  try {
    app.run();
  }
  catch (const std::exception &e) {
    LOG_ERROR("App") << e.what();
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}


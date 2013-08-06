#include <list>
#include <string>

// Returns true is some service is current listerning on the given port and false otherwise
bool isPortUsed(int port);

std::list<std::string> getAllHostnames();

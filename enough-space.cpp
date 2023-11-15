#include <iostream>
#include <sys/statvfs.h>

bool hasEnoughSpace(const std::string &path, size_t requiredBytes) {
  struct statvfs stat;

  if (statvfs(path.c_str(), &stat) != 0) {
    // Error occurred while getting file system statistics
    std::cerr << "Error getting file system statistics for " << path
              << std::endl;
    return false;
  }

  // Calculate the available space in bytes
  size_t availableBytes = stat.f_bavail * stat.f_frsize;

  // Check if there is enough space
  std::cout << availableBytes << std::endl;
  std::cout << requiredBytes << std::endl;
  if (availableBytes >= requiredBytes) {
    std::cout << "There is enough space on the disk." << std::endl;
    return true;
  } else {
    std::cout << "Not enough space on the disk." << std::endl;
    return false;
  }
}

int main() {
  std::string path = "./"; // Replace with the actual path
  size_t requiredBytes =
      1024 * 1024 * 100; // Replace with the required number of bytes

  if (hasEnoughSpace(path, requiredBytes)) {
    // Perform your operations that require enough disk space
    std::cout << "Performing operations that require enough disk space..."
              << std::endl;
  } else {
    // Handle the case where there is not enough space
    std::cerr << "Cannot perform operations due to insufficient disk space."
              << std::endl;
  }

  return 0;
}

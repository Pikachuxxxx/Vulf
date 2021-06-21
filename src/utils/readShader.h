#include <fstream>

static std::vector<char> readFile(const std::string& path)
{
    // Open the file in binay mode with the cursor at the end of the file
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    // Determine the size of the file to create the buffer to hold the daat
    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    // Now seek to beginning to read the entire data at once
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    // Close the file and return the data
    file.close();
    return buffer;
}

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

std::string read_file(const char* fileName) {
    std::ifstream file(fileName);
    if (!file.is_open()) {
        std::cout << "The file " << fileName << " could no be open\n";
        return "";
    }

    file.seekg(0, std::ios_base::end);
    std::streampos fileSize = file.tellg();
    if (fileSize.state()._State & std::ios_base::failbit) {
        std::cout << "An error occured reading the file " << fileName << '\n';
        return "";
    }
    file.seekg(0, std::ios_base::beg);

    std::string contents{};
    contents.resize(static_cast<std::size_t>(fileSize) + 1);
    file.read(&contents[0], fileSize);
    return contents;
}

std::vector<std::string> split_str(std::string& str, const std::string& sep) {
    std::vector<std::string> parts{};
    std::size_t front{}, tail{};

    while ((tail = str.find(sep, front)) != std::string::npos) {
        std::string piece = str.substr(front, tail - front + sep.size());
        parts.emplace_back(std::move(piece));
        front = tail + sep.size();
    }
    parts.emplace_back(str.substr(front));

    return parts;
}


int main(int argc, char* argv[]) {
    auto file = "nbt.txt";
    std::string nbt = read_file(file);
    std::cout << "test\n";
    std::cout << nbt;
    return 0;
}

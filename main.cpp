#include <algorithm>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

static const std::regex oreRgx(R"_(translate".*mineral\.(\w+)")_");
static const std::regex xRgx(R"_(dimension:.*x:(-?\d+))_");
static const std::regex zRgx(R"_(dimension:.*z:(-?\d+))_");

enum mc_color {
    black = 0,
    dark_blue,
    dark_green,
    dark_aqua,
    dark_red,
    dark_purple,
    gold,
    gray,
    dark_grey,
    blue,
    green,
    aqua,
    red,
    light_purple,
    yellow,
    white
};

std::unordered_map<mc_color, std::vector<std::string>> color_map{
    { black, { "Lignite", "Bituminous" } },
    { dark_blue, { "Graphite" } },
    { dark_green, { "Malachite", "Bismuthinite" } },
    { dark_aqua, { "Kimberlite" } },
    { dark_red, { "Cinnabar", "Hematite" } },
    { dark_purple, { "Archaic" } },
    { gold, { "Gold", "Mephitic", "Sulfur" } },
    { gray, { "Silver", "Cryolite", "Saltpeter" } },
    { dark_grey, { "Magnetite", "Tetrahedrite" } },
    { blue, { "Lapis", "Certus" } },
    { green, { "Uraninite", "Emerald" } },
    { aqua, { "Garnierite" } },
    { red, { "Copper" } },
    { light_purple, { "Kaolinite", "Sylvite" } },
    { yellow, { "Sphalerite", "Gypsum", "Limonite" } },
    { white, { "Cassiterite", "Halite", "Borax" } },
};

std::unordered_map<std::string, std::vector<std::string>> category_map{
    { "Coal", { "Lignite", "Bituminous", "Archaic" } },
    { "Copper", { "Copper", "Tetrahedrite", "Malachite" } },
    { "Bismuth", { "Bismuthinite" } },
    { "Redstone", { "Cinnabar" } },
    { "Nickel", { "Garnierite" } },
    { "Gold", { "Gold", "Mephitic" } },
    { "Silver", { "Silver" } },
    { "Iron", { "Hematite", "Magnetite", "Limonite" } },
    { "Uranium", { "Uraninite" } },
    { "Gems", { "Kimberlite", "Emerald", "Certus", "Lapis" } },
    { "Aluminium", { "Cryolite" } },
    { "Minerals", { "Kaolinite", "Graphite", "Gypsum", "Sylvite", "Borax", "Halite", "Sulfur", "Saltpeter" } },
    { "Zinc", { "Sphalerite" } },
    { "Tin", { "Cassiterite" } },
};

std::string to_lower(std::string str);
std::string fmt_ore_name(std::string raw_name, const std::string& cat);

template <typename Result, typename Collection>
Result get_if_any(const std::string& id, Collection collection, Result default_r) {
    std::string lc_id{ to_lower(id) };
    for (const auto& entry : collection) {
        const auto& values = entry.second;
        const bool any = std::any_of(values.begin(), values.end(), [&](const std::string& value) {
            return lc_id.find(to_lower(value)) != std::string::npos;
        });
        if (any)
            return entry.first;
    }
    return default_r;
}

struct Waypoint {

    Waypoint(std::string raw_name, std::string x_p, std::string z_p) : x(std::move(x_p)), z(std::move(z_p)) {
        category = get_if_any(raw_name, category_map, std::string{ "Unkown" });
        oreName = fmt_ore_name(raw_name, category);
        color = get_if_any(raw_name, color_map, mc_color::red);
    }

    std::string get_initials() {
        auto it = oreName.find_last_of(' ');
        std::string initials{ oreName.begin() + it + 1, oreName.begin() + it + 3 };
        return initials;
    }

    static std::string fmt_ore_name(std::string raw_name, const std::string& cat) {
        const std::string ore_group = '(' + cat + ')';
        std::size_t last_under = raw_name.find_last_of('_');
        raw_name[last_under] = '[';
        raw_name.push_back(']');

        char& l = raw_name[last_under + 1];
        l = std::toupper(l);

        std::string::size_type under = raw_name.find('_');
        if (under != std::string::npos) {
            raw_name[under] = ' ';
            char& m = raw_name[under + 1];
            m = std::toupper(m);
        }

        char& f = raw_name.front();
        f = std::toupper(f);

        raw_name.reserve(raw_name.size() + ore_group.size() + 1);
        raw_name.append(ore_group);
        raw_name.push_back(' ');

        std::rotate(raw_name.begin(), raw_name.end() - 1 - ore_group.size(), raw_name.end());
        return raw_name;
    }

    operator std::string() {
        std::ostringstream os{};
        // waypoint:name:initials:x:y:z:color:disabled:type:set:rotate_on_tp:tp_yaw:visibility_type:destination
        os << "waypoint:" << oreName << ':' << get_initials() << ':' << x << ":70:" << z << ':' << std::to_string(color)
           << ":false:0:Mineral Survey:false:0:0:false";
        return os.str();
    }

    std::string oreName, category, x, z;
    mc_color color;
};

std::string to_lower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
    return str;
}

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

std::vector<std::string>& get_waypoints(std::vector<std::string>& strVec) {
    std::transform(strVec.begin(), strVec.end(), strVec.begin(), [&](std::string& s) -> std::string {
        try {
            using std::regex_search;
            std::smatch ore, x, z;
            if (regex_search(s, ore, oreRgx) && regex_search(s, x, xRgx) && regex_search(s, z, zRgx)) {
                return { Waypoint(ore[1], x[1], z[1]) };
            } else {
                return {};
            }
        } catch (const std::regex_error& ex) {
            std::cout << ex.what() << std::endl;
            return {};
        }
    });

    strVec.erase(std::remove_if(strVec.begin(), strVec.end(), [](auto& s) { return s.empty(); }), strVec.end());
    return strVec;
}

int main(int argc, char* argv[]) {
    const char* file = "nbt.txt";
    std::string nbt = read_file(file);
    nbt.erase(std::remove_if(nbt.begin(), nbt.end(), ::isspace), nbt.end());
    auto waypoints = get_waypoints(split_str(nbt, "data:"));
    std::ofstream output("out.txt");
    for (auto& w : waypoints) {
        output << w << '\n';
    }
    std::cout << "Finished" << std::endl;
    return 0;
}

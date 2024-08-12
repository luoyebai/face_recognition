// std
#include <filesystem>

// config reader
#include "config_reader.hpp"

namespace cv {
void ReadBool(const cv::FileNode &node, bool &value,
              const bool &default_value) {
    std::string s(static_cast<std::string>(node));
    if (s == "y" || s == "Y" || s == "yes" || s == "Yes" || s == "YES" ||
        s == "true" || s == "True" || s == "TRUE" || s == "on" || s == "On" ||
        s == "ON") {
        value = true;
        return;
    }
    if (s == "n" || s == "N" || s == "no" || s == "No" || s == "NO" ||
        s == "false" || s == "False" || s == "FALSE" || s == "off" ||
        s == "Off" || s == "OFF") {
        value = false;
        return;
    }
    value = static_cast<int>(node);
}
} // namespace cv

void ConfigReader::setConfigPath(std::string dir_path) {
    if (std::filesystem::exists(dir_path)) {
        if (std::filesystem::is_directory(dir_path)) {
            config_path_ = dir_path;
            std::cout << "[readconfig->setConfigPath]:"
                         "设置路径<"
                      << dir_path << ">成功\n";
        } else {
            std::cout << dir_path << "[readconfig->setConfigPath]:<" << dir_path
                      << ">貌似不是文件夹\n";
        }
    } else {
        std::cout << dir_path << "[readconfig->setConfigPath]:<" << dir_path
                  << ">貌似不存在\n";
    }
}

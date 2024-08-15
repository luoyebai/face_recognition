#pragma once

// std
#include <chrono>
#include <filesystem>
#include <iostream>
#include <thread>

// opencv
#include <opencv2/core/persistence.hpp>

namespace cv {
/**
 * @brief 定义新的bool类型解析器
 *
 * @param node
 * @param value
 * @param default_value
 */
void ReadBool(const cv::FileNode &node, bool &value, const bool &default_value);

/**
 * @brief 特化bool类型>>操作运算符
 *
 * @tparam
 * @param n
 * @param value
 */
template <> inline void operator>>(const cv::FileNode &n, bool &value) {
    ReadBool(n, value, false);
}
} // namespace cv

/**
 * @brief 配置读取器
 *
 */
class ConfigReader {
  public:
    /**
     * @brief 设置配置文件文件夹
     *
     * @param dir_path 文件夹路径
     */
    void setConfigPath(std::string dir_path);

    /**
     * @brief 读取配置文件中指定数据
     *
     * @tparam T 数据类型
     * @param file_name 文件名
     * @param data_name 数据名
     * @param default_val 默认值
     * @return auto
     */
    template <typename T>
    auto readData(std::string file_name, std::string data_name,
                  T default_val) const {
        auto file_path = config_path_ + file_name;

        if (!std::filesystem::exists(file_path)) {
            std::cerr << "[readconfig->readData]:<" << file_path
                      << ">貌似不存在\n";
            return default_val;
        }

        cv::FileStorage fs(file_path, cv::FileStorage::READ);
        if (!fs.isOpened()) {
            std::cerr << "[readconfig->readData]:"
                      << "打开<" << file_path
                      << ">文件失败，请检查文件格式，如是否在开头指定版本等\n";
            return default_val;
        }
        T data;
        fs[data_name] >> data;
        fs.release();
        return data;
    }

    /**
     * @brief 热更新，配置文件修改时，执行回调函数
     *
     * @tparam CallBack 回调函数类型
     * @param file_name 文件名
     * @param f 回调函数
     * @param fps 刷新频率
     * @return true 注册成功
     * @return false 注册失败
     */
    template <typename CallBack>
    bool registerHotUpdate(const std::string &file_name, CallBack &&f,
                           size_t fps = 10) const {
        auto file_path = config_path_ + file_name;
        if (!std::filesystem::exists(file_path))
            return false;
        std::thread([file_path, f, fps] {
            auto last_time = std::filesystem::last_write_time(file_path);
            while (true) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(1000 / fps));
                auto now_time = std::filesystem::last_write_time(file_path);
                if (last_time.time_since_epoch() == now_time.time_since_epoch())
                    continue;
                last_time = std::filesystem::last_write_time(file_path);
                f();
            }
        }).detach();
        return true;
    }

  private:
    // 默认配置文件文件夹路径
    std::string config_path_ = __CONFIG_DIR__;
};

/**
 * @brief 读取配置文件中的数据
 *
 * @tparam  对const char*特化
 * @param file_name 文件名
 * @param data_name 数据名
 * @param default_val 默认值
 * @return auto
 */
template <>
inline auto ConfigReader::readData(std::string file_name, std::string data_name,
                                   const char *default_val) const {
    return readData<std::string>(file_name, data_name,
                                 std::string(default_val));
}

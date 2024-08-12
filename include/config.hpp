#pragma once

#include <any>
#include <map>

#include "config_reader.hpp"

constexpr const char *kconfig_name = "val.yml";

// 配置数据类型
using ConfigBasicType = std::any;
using ConfigMap = std::map<std::string, ConfigBasicType>;

// 配置数据表包含数据名和默认值
const ConfigMap kconfig_map = {
    {"debug", 1},
    {"cap_or_video", 0},
    {"cap_index", 0},
    {"video_name", std::string("face_test.mp4")},
    {"zoom", 1.0f},
    {"backend_target", 0},
    {"detection_onnx", std::string("face_detection_yunet_2023mar.onnx")},
    {"sface_onnx", std::string("face_recognition_sface_2021dec.onnx")},
    {"detect_threshold", 0.8f},
    {"nms_threshold", 0.3f},
    {"top_k", 5000},
    {"distance_type", 0},
    {"cosine_threshold", 0.363f},
    {"norml2_threshold", 1.128f},
    {"targets_dir_name", std::string("targets")},
    {"draw_face_points", true},
};

/**
 * @brief 配置基本数据转为原始数据
 * 对原始数据类型进行强匹配如：const char* 不能 -> std::string 反之亦然
 *
 * @tparam T 原始数据类型
 * @param data 配置基本类型数据
 * @return auto 数据
 */
template <typename T> auto ConfigBasicTypeToRaw(ConfigBasicType data) {
    std::any data_copy = data;
    if (data.has_value())
        return std::any_cast<T>(data_copy);
    else
        return T();
}

/**
 * @brief 传入读取器，读取指定配置数据
 *
 * @tparam T 配置数据类型
 * @param reader 读取器
 * @param data_name 数据名
 * @return T 数据
 */
template <typename T>
T GetConfigData(const ConfigReader &reader, const std::string &data_name) {
    auto raw_data = ConfigBasicTypeToRaw<T>(kconfig_map.at(data_name));
    return reader.readData(kconfig_name, data_name, raw_data);
}

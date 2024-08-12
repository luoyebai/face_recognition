#include "config.hpp"
#include "config_reader.hpp"
#include "detector.hpp"

/**
 * @brief 构造YuNet
 *
 * @param reader 配置读取器
 * @return YuNet
 */
YuNet GetYuNet(const ConfigReader &reader) {
    auto yunet_model_path =
        __DATA_DIR__ + GetConfigData<std::string>(reader, "detection_onnx");
    auto detect_threshold = GetConfigData<float>(reader, "detect_threshold");
    auto nms_threshold = GetConfigData<float>(reader, "nms_threshold");
    auto top_k = GetConfigData<int>(reader, "top_k");
    auto backend_target = GetConfigData<int>(reader, "backend_target");
    const int backend_id = backend_target_pairs.at(backend_target).first;
    const int target_id = backend_target_pairs.at(backend_target).second;
    auto yunet = YuNet(yunet_model_path, cv::Size(320, 320), detect_threshold,
                       nms_threshold, top_k, backend_id, target_id);
    return yunet;
}

/**
 * @brief 构造SFace
 *
 * @param reader  配置读取器
 * @return SFace
 */
SFace GetSFace(const ConfigReader &reader) {
    auto sface_model_path =
        __DATA_DIR__ + GetConfigData<std::string>(reader, "sface_onnx");
    auto backend_target = GetConfigData<int>(reader, "backend_target");
    const int backend_id = backend_target_pairs.at(backend_target).first;
    const int target_id = backend_target_pairs.at(backend_target).second;
    auto distance_type = GetConfigData<int>(reader, "distance_type");
    auto cosine_threshold = GetConfigData<float>(reader, "cosine_threshold");
    auto norml2_threshold = GetConfigData<float>(reader, "norml2_threshold");
    auto sface = SFace(sface_model_path, backend_id, target_id, distance_type);
    sface.setThresholdCosine(cosine_threshold);
    sface.setThresholdNorml2(norml2_threshold);
    return sface;
}

/**
 * @brief 获得所有的识别目标数据
 *
 * @param reader 配置读取器
 * @param detector_ptr 完整识别器的指针
 * @return TargetDataVec
 */
TargetDataVec GetAllTargetData(const ConfigReader &reader,
                               cv::Ptr<Detector> detector_ptr) {
    std::vector<std::string> targets_path;
    TargetDataVec target_data_vec;
    auto targets_dir_path =
        __DATA_DIR__ + GetConfigData<std::string>(reader, "targets_dir_name");
    cv::glob(targets_dir_path, targets_path, false);
    for (const auto &target_path : targets_path) {
        std::string::size_type pos = target_path.find_last_of('/') + 1;
        std::string filename =
            target_path.substr(pos, target_path.length() - pos - 4);
        auto target_image = cv::imread(target_path);
        auto detect_result = detector_ptr->detectFace(target_image);
        if (detect_result.faces.empty())
            std::cerr << "未检测到" << filename << "人脸\n";
        else
            target_data_vec.push_back({filename, detect_result.features[0]});
    }
    return target_data_vec;
}

int main() {
    // 读取配置
    ConfigReader reader;
    auto yunet = GetYuNet(reader);
    auto sface = GetSFace(reader);
    // 初始化识别器
    cv::Ptr<Detector> detector_ptr = cv::makePtr<Detector>(yunet, sface);
    // 初始化目标数据
    auto target_data_vec = GetAllTargetData(reader, detector_ptr);
    // 目标数据加入识别器
    detector_ptr->addTargetDatas(target_data_vec);

    // 初始化视频流
    auto cap_or_video = GetConfigData<int>(reader, "cap_or_video");
    assert((cap_or_video == 0 || cap_or_video == 1) &&
           "cap_or_video 必须是 0 或者 1");

    cv::VideoCapture video_capture;
    cv::Mat input;

    if (cap_or_video == 0) {
        auto cap_index = GetConfigData<int>(reader, "cap_index");
        video_capture.open(cap_index);
    }
    if (cap_or_video == 1) {
        auto video_path =
            __DATA_DIR__ + GetConfigData<std::string>(reader, "video_name");
        video_capture.open(video_path);
    }

    auto debug = GetConfigData<int>(reader, "debug");
    auto top_k = GetConfigData<int>(reader, "top_k");
    auto zoom = GetConfigData<float>(reader, "zoom");
    auto draw_face_points = GetConfigData<bool>(reader, "draw_face_points");

    cv::TickMeter tick_meter_video;
    cv::TickMeter tick_meter_detect;

    // 注册热更新
    reader.registerHotUpdate(
        kconfig_name, [&reader, &debug, &zoom, &top_k, &draw_face_points]() {
            debug = GetConfigData<int>(reader, "debug");
            top_k = GetConfigData<int>(reader, "top_k");
            zoom = GetConfigData<float>(reader, "zoom");
            draw_face_points = GetConfigData<bool>(reader, "draw_face_points");
        });

    while (cv::waitKey(1) != 'q') {
        tick_meter_detect.start();
        tick_meter_video.start();
        // 读一帧
        if (!video_capture.read(input))
            break;
        cv::resize(input, input,
                   cv::Size(input.cols * zoom, input.rows * zoom));
        tick_meter_video.stop();

        // 获取图片所有识别到的人脸特征等数据
        auto detect_result = detector_ptr->detectFace(input, top_k);
        auto match_data_vec = detector_ptr->matchTargetFace(detect_result);
        tick_meter_detect.stop();

        const auto video_fps = static_cast<float>(tick_meter_video.getFPS());
        const auto detect_fps = static_cast<float>(tick_meter_detect.getFPS());

        if (debug) {
            auto output_image =
                visualize(input, match_data_vec,
                          cv::format("FPS:%.2f/%.2f", detect_fps, video_fps),
                          draw_face_points);
            cv::imshow("main", output_image);
        }
        tick_meter_video.reset();
        tick_meter_detect.reset();
    }
}

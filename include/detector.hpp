#pragma once
// std
#include <cstdio>
#include <string>

// opencv
#include "opencv2/core/cvstd_wrapper.hpp"
#include <opencv2/core/types.hpp>
#include <opencv2/objdetect/face.hpp>
#include <opencv2/opencv.hpp>

/**
 * @brief 识别器后端处理方式表
 *
 */
const std::vector<std::pair<int, int>> backend_target_pairs = {
    {cv::dnn::DNN_BACKEND_OPENCV, cv::dnn::DNN_TARGET_CPU},
    {cv::dnn::DNN_BACKEND_CUDA, cv::dnn::DNN_TARGET_CUDA},
    {cv::dnn::DNN_BACKEND_CUDA, cv::dnn::DNN_TARGET_CUDA_FP16},
    {cv::dnn::DNN_BACKEND_TIMVX, cv::dnn::DNN_TARGET_NPU},
    {cv::dnn::DNN_BACKEND_CANN, cv::dnn::DNN_TARGET_NPU}};

/**
 * @brief YuNet模型人脸检测器
 *
 */
class YuNet {
  public:
    YuNet(const std::string &model_path, const cv::Size &input_size,
          const float conf_threshold, const float nms_threshold,
          const int top_k, const int backend_id, const int target_id) {

        detector_ = cv::FaceDetectorYN::create(model_path, "", input_size,
                                               conf_threshold, nms_threshold,
                                               top_k, backend_id, target_id);
    }

    /**
     * @brief 设置输入图像大小
     *
     * @param input_size
     */
    void setInputSize(const cv::Size &input_size);

    /**
     * @brief 设置最多识别到几个人
     *
     * @param top_k
     */
    void setTopK(const int top_k);

    /**
     * @brief 识别
     *
     * @param image 输入图像
     * @return cv::Mat 返回检测到的所有人脸位置
     */
    cv::Mat infer(const cv::Mat &image);

  private:
    cv::Ptr<cv::FaceDetectorYN> detector_;
};

/**
 * @brief SFace模型人脸特征提取比较器
 *
 */
class SFace {
  public:
    SFace(const std::string &model_path, const int backend_id,
          const int target_id, const int distance_type)
        : distance_type_(
              static_cast<cv::FaceRecognizerSF::DisType>(distance_type)) {
        recognizer_ =
            cv::FaceRecognizerSF::create(model_path, "", backend_id, target_id);
    }

    /**
     * @brief 计算人脸特征数据
     *
     * @param orig_image 原始图像
     * @param face_image 人脸图像
     * @return cv::Mat 特征数据
     */
    cv::Mat extractFeatures(const cv::Mat &orig_image,
                            const cv::Mat &face_image);

    /**
     * @brief 匹配目标特征量
     *
     * @param target_features 目标特征量
     * @param query_features 输入特征量
     * @return std::pair<double, bool> 相似度，是否匹配成功
     */
    std::pair<double, bool> matchFeatures(const cv::Mat &target_features,
                                          const cv::Mat &query_features);

    /**
     * @brief 设置阈值
     *
     * @param cosine_threshold
     */
    void setThresholdCosine(float cosine_threshold);

    /**
     * @brief 设置阈值
     *
     * @param cosine_threshold
     */
    void setThresholdNorml2(float norml2_threshold);

  private:
    cv::Ptr<cv::FaceRecognizerSF> recognizer_;
    cv::FaceRecognizerSF::DisType distance_type_;
    float threshold_cosine_ = 0.363f;
    float threshold_norml2_ = 1.128f;
};

/**
 * @brief 人脸检测结果类型，包括人脸和特征值
 *
 */
struct DetectResult {
    cv::Mat faces;
    std::vector<cv::Mat> features;
};

/**
 * @brief 目标特征值类型，包括名字和特征值
 *
 */
struct TargetData {
    std::string name;
    cv::Mat feature;
};

/**
 * @brief 匹配的结果，包括名字、人脸框、置信度、是否匹配
 *
 */
struct MatchData {
    std::string name = "?";
    cv::Mat face;
    float conf = 0.f;
    bool match = false;
};

using TargetDataVec = std::vector<TargetData>;
using MatchDataVec = std::vector<MatchData>;

/**
 * @brief 完整的识别器
 *
 */
class Detector {
  public:
    explicit Detector(YuNet yunet, SFace sface) {
        yunet_ptr_ = cv::makePtr<YuNet>(yunet);
        sface_ptr_ = cv::makePtr<SFace>(sface);
    }

    /**
     * @brief 添加目标特征值
     *
     * @param new_target_data 新的目标数据
     */
    void addTargetData(const TargetData &new_target_data);

    /**
     * @brief 批量添加目标特征值
     *
     * @param new_target_data_vec 新的目标数据向量
     */
    void addTargetDatas(const TargetDataVec &new_target_data_vec);

    /**
     * @brief 删除所有的目标
     *
     */
    void clearTargetDatas();

    /**
     * @brief 人脸识别，获得一张图片上所有的人脸和对应特征值
     *
     * @param input 输入图像
     * @param top_k 最多几张人脸
     * @return DetectResult 结果
     */
    DetectResult detectFace(const cv::Mat input, int top_k = 1);

    /**
     * @brief 使用识别到的数据进行人脸匹配
     *
     * @param detect_result 识别到的人脸
     * @return MatchDataVec 匹配的结果
     */
    MatchDataVec matchTargetFace(DetectResult detect_result);

  private:
    TargetDataVec target_data_vec_;
    cv::Ptr<YuNet> yunet_ptr_ = nullptr;
    cv::Ptr<SFace> sface_ptr_ = nullptr;
};

/**
 * @brief 可视化匹配结果
 *
 * @param image 输入图像
 * @param match_data_vec 匹配到的结果
 * @param fps_text 显示帧率
 * @return cv::Mat
 */
cv::Mat visualize(const cv::Mat &image, MatchDataVec match_data_vec,
                  const std::string &fps_text = "",
                  bool draw_face_points = true);

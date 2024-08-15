#include "detector.hpp"

void YuNet::setInputSize(const cv::Size &input_size) {
    detector_->setInputSize(input_size);
    return;
}
void YuNet::setTopK(const int top_k) {
    detector_->setTopK(top_k);
    return;
}

cv::Mat YuNet::infer(const cv::Mat &image) {
    cv::Mat result;
    detector_->detect(image, result);
    return result;
}

void SFace::setThresholdCosine(float cosine_threshold) {
    threshold_cosine_ = cosine_threshold;
    return;
}

void SFace::setThresholdNorml2(float norml2_threshold) {
    threshold_norml2_ = norml2_threshold;
    return;
}

cv::Mat SFace::extractFeatures(const cv::Mat &orig_image,
                               const cv::Mat &face_image) {
    // Align and crop detected face from original image
    cv::Mat target_aligned;
    recognizer_->alignCrop(orig_image, face_image, target_aligned);
    // Extract features from cropped detected face
    cv::Mat target_features;
    recognizer_->feature(target_aligned, target_features);
    return target_features.clone();
}

std::pair<double, bool> SFace::matchFeatures(const cv::Mat &target_features,
                                             const cv::Mat &query_features) {
    const double score =
        recognizer_->match(target_features, query_features, distance_type_);
    if (distance_type_ == cv::FaceRecognizerSF::DisType::FR_COSINE) {
        return {score, score >= threshold_cosine_};
    }
    return {score, score <= threshold_norml2_};
}

// 添加目标特征值
void Detector::addTargetData(const TargetData &new_target_data) {
    target_data_vec_.push_back(new_target_data);
    return;
}

// 批量添加目标特征值
void Detector::addTargetDatas(const TargetDataVec &new_target_data_vec) {
    for (auto new_target_data : new_target_data_vec)
        target_data_vec_.push_back(new_target_data);
    return;
}

void Detector::clearTargetDatas() {
    target_data_vec_.clear();
    return;
}

// 人脸识别，获得一张图片上所有的人脸和对应特征值
DetectResult Detector::detectFace(const cv::Mat input, int top_k) {
    yunet_ptr_->setInputSize(input.size());
    yunet_ptr_->setTopK(top_k);
    // 人脸
    cv::Mat faces = yunet_ptr_->infer(input);
    // 特征值
    std::vector<cv::Mat> features;
    for (size_t i = 0; i < faces.rows; ++i) {
        cv::Mat feature = sface_ptr_->extractFeatures(input, faces.row(i));
        features.push_back(feature);
    }
    return DetectResult(faces, features);
}

// 匹配人脸
MatchDataVec Detector::matchTargetFace(DetectResult detect_result) {
    MatchDataVec match_data_vec;
    for (size_t i = 0; i < detect_result.faces.rows; ++i) {
        MatchData match_data;
        match_data.face = detect_result.faces.row(i);
        for (auto target_data : target_data_vec_) {
            auto match_raw_data = sface_ptr_->matchFeatures(
                target_data.feature, detect_result.features[i]);
            if (match_raw_data.first <= match_data.conf)
                continue;
            match_data.conf = match_raw_data.first;
            match_data.match = match_raw_data.second;
            match_data.name = target_data.name;
        }
        match_data_vec.push_back(match_data);
    }
    return match_data_vec;
}

void DrawFacePoint(cv::Mat input, const cv::Mat &face) {
    static const std::vector<cv::Scalar> landmark_color{
        cv::Scalar(255, 0, 0),   // right eye
        cv::Scalar(0, 0, 255),   // left eye
        cv::Scalar(0, 255, 0),   // nose tip
        cv::Scalar(255, 0, 255), // right mouth corner
        cv::Scalar(0, 255, 255)  // left mouth corner
    };

    // Draw landmarks
    for (int i = 0; i < landmark_color.size(); ++i) {
        int x = static_cast<int>(face.at<float>(2 * i + 4)),
            y = static_cast<int>(face.at<float>(2 * i + 5));
        cv::circle(input, cv::Point(x, y), 2, landmark_color[i], 2);
    }
}

cv::Mat visualize(const cv::Mat &image, MatchDataVec match_data_vec,
                  const std::string &fps_text, bool draw_face_points) {
    static const cv::Scalar green_color{0, 255, 0};
    static const cv::Scalar red_color{0, 0, 255};
    auto output_image = image.clone();
    cv::putText(output_image, fps_text, cv::Point(0, 15),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, green_color, 2);
    for (auto match_data : match_data_vec) {
        auto [name, face, conf, match] = match_data;
        auto color = match ? green_color : red_color;
        int x = static_cast<int>(face.at<float>(0));
        int y = static_cast<int>(face.at<float>(1));
        int w = static_cast<int>(face.at<float>(2));
        int h = static_cast<int>(face.at<float>(3));
        cv::putText(output_image, name, cv::Point(x, y + 12),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, color, 2);
        cv::putText(output_image, cv::format("%.2f", conf),
                    cv::Point(x, y + 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, color,
                    2);
        cv::rectangle(output_image, cv::Rect(x, y, w, h), color, 2);
        if (draw_face_points)
            DrawFacePoint(output_image, face);
    }
    return output_image;
}

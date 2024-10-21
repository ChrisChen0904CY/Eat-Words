#include "utils.h"
#include <cmath>
#include <fstream>
#include <QDebug>
#include <Windows.h>

/**

* @brief Loading in the vocabulary for the Bert-Model
* @author Chris Chan
* @date 2024/10/20

*/
std::wstring string2wstring(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], size_needed);
    return wstr;
}

std::unordered_map<std::wstring, int> load_vocab(const std::string& vocab_file) {
    std::unordered_map<std::wstring, int> vocab;
    std::ifstream infile(vocab_file);
    std::string line;
    int index = 0;
    while (getline(infile, line)) {
        vocab[string2wstring(line)] = index++;
    }
    return vocab;
}

std::vector<int64_t> tokenize(const std::wstring& text,
                              const std::unordered_map<std::wstring, int>& vocab) {
    std::vector<int64_t> input_ids;
    // Process each wide character (supports Chinese characters)
    for (wchar_t ch : text) {
        // Convert wchar_t to wstring
        std::wstring token(1, ch);
        if (vocab.find(token) != vocab.end()) {
            input_ids.push_back(static_cast<int64_t>(vocab.at(token)));
        } else {
            if (vocab.find(L"[UNK]") != vocab.end()) {
                input_ids.push_back(static_cast<int64_t>(vocab.at(L"[UNK]")));
            } else {
                return {};
            }
        }
    }
    return input_ids;
}

/**

* @brief Compute aid functions
* @author Chris Chan
* @date 2024/10/20

*/
// Cos Similarity
// Cosine Similarity with zero padding
float cosine_similarity(const std::vector<float>& vec1, const std::vector<float>& vec2) {
    size_t max_size = max(vec1.size(), vec2.size());
    std::vector<float> padded_vec1(max_size, 0.0f);
    std::vector<float> padded_vec2(max_size, 0.0f);

    // Copy elements and pad with zeros
    for (size_t i = 0; i < vec1.size(); ++i) {
        padded_vec1[i] = vec1[i];
    }
    for (size_t i = 0; i < vec2.size(); ++i) {
        padded_vec2[i] = vec2[i];
    }

    float dot_product = 0.0f;
    float norm_a = 0.0f;
    float norm_b = 0.0f;

    for (size_t i = 0; i < max_size; i++) {
        dot_product += padded_vec1[i] * padded_vec2[i];
        norm_a += padded_vec1[i] * padded_vec1[i];
        norm_b += padded_vec2[i] * padded_vec2[i];
    }

    // Avoid division by zero
    if (norm_a == 0.0f || norm_b == 0.0f) {
        throw std::runtime_error("One or both vectors are zero vectors");
    }

    return dot_product / (sqrt(norm_a) * sqrt(norm_b));
}

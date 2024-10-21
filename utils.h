#ifndef UTILS_H
#define UTILS_H

#include <unordered_map>
#include <vector>
#include <string>

/**

* @brief Loading in the vocabulary for the Bert-Model
* @author Chris Chan
* @date 2024/10/20

*/
std::wstring string2wstring(const std::string& str);
std::unordered_map<std::wstring, int> load_vocab(const std::string& vocab_file);
std::vector<int64_t> tokenize(const std::wstring& text,
                              const std::unordered_map<std::wstring, int>& vocab);

/**

* @brief Compute aid functions
* @author Chris Chan
* @date 2024/10/20

*/
// Cos Similarity
float cosine_similarity(const std::vector<float>& vec1, const std::vector<float>& vec2);

#endif // UTILS_H

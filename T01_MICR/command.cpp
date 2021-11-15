#include "command.h"

/* Splits string(text) into substrings(words) divided by given token */
void split_string_by_token(const std::string &text, char token, std::vector<std::string> &words) {
  for (size_t i = 0, prev = 0; i <= text.size(); i++)
    if (text[i] == token || text[i] == '\0') {
      if (i == prev) {
        prev++;
        if (token == '/' && i == 0)
          words.emplace_back("");
      }
      else {
        words.emplace_back(text.substr(prev, i - prev));
        prev = i + 1;
      }
    }
}
#ifndef UTILITIES_H
#define UTILITIES_H

#include <string>
#include <vector>

bool isNumber(std::string str);

bool isInterval(std::string str);

bool isPageValid(int pageIndex, int totalPages);

bool isPageIntervalValid(int leftBorder, int rightBorder, int totalPages);

std::vector<std::string> splitString(std::string initStr);

#endif

#ifndef UTILITIES_H
#define UTILITIES_H

#include<string>

bool isNumber(std::string str);

bool isInterval(std::string str);

bool isPageValid(int pageIndex, int totalPages);

bool isPageIntervalValid(int leftBorder, int rightBorder, int totalPages);

#endif

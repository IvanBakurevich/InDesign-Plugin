#include "VCPlugInHeaders.h"
#include "Utilities.h";

bool isNumber(std::string str) {
	for (int i = 0; i < str.size(); i++) {
		if (str[i] < '0' || str[i] > '9') {
			return false;
		}
	}
	return true;
}

bool isInterval(std::string str) {
	for (int i = 0; i < str.size(); i++) {
		if ((str[i] < '0' || str[i] > '9') && str[i] != '-') {
			return false;
		}
	}
	return true;
}


bool isPageValid(int pageIndex, int totalPages) {
	if (pageIndex <= 0 || pageIndex > totalPages) {
		return false;
	}
	return true;
}

bool isPageIntervalValid(int leftBorder, int rightBorder, int totalPages) {
	if (leftBorder >= rightBorder) {
		return false;
	}
	if (!isPageValid(leftBorder, totalPages) || !isPageValid(rightBorder, totalPages)) {
		return false;
	}
	return true;
}

std::vector<std::string> splitString(std::string initStr) {
	std::vector<std::string> vector;

	initStr.erase(std::remove(initStr.begin(), initStr.end(), ' '), initStr.end());

	std::stringstream ss(initStr);
	std::string itemStr;
	while (getline(ss, itemStr, ',')) {
		vector.push_back(itemStr);
	}
	return vector;
}
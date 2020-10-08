#include "Utilities.h";
#include "VCPlugInHeaders.h"

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
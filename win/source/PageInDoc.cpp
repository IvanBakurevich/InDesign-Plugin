#include "VCPlugInHeaders.h"
#include "PageInDoc.h"


PageInDoc::PageInDoc(int32 spreadIndex, int32 inSpreadIndex)
{
	this->spreadIndex = spreadIndex;
	this->inSpreadIndex = inSpreadIndex;
}

PageInDoc::PageInDoc(int32 absPageIndex, std::vector<int32> spreadPagesIntervals)
{
	int32 spreadIndex = -1;
	int32 inSpreadIndex = -1;

	int32 index = 0;
	int32 prevValue = 0;
	for (auto it = spreadPagesIntervals.begin(); it != spreadPagesIntervals.end(); it++,index++) {
		if (*it - absPageIndex >= 0) {
			spreadIndex = index;
			inSpreadIndex = absPageIndex - prevValue - 1;
			break;
		}
		prevValue = *it;
	}
	this->spreadIndex = spreadIndex;
	this->inSpreadIndex = inSpreadIndex;
}

int32 PageInDoc::getSpreadIndex()
{
	return this->spreadIndex;
}

int32 PageInDoc::getInSpreadIndex()
{
	return this->inSpreadIndex;
}

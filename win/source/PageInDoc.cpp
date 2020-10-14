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
	for (int32 i = 0; i < spreadPagesIntervals.size(); i++) {
		if (spreadPagesIntervals[i] - absPageIndex >= 0) {
			spreadIndex = i;
			int prevSpreadIndex = 0;
			if (i != 0) {
				prevSpreadIndex = spreadPagesIntervals[i - 1];
			}
			inSpreadIndex = absPageIndex - prevSpreadIndex - 1;
		}
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

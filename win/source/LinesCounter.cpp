#include "VCPlugInHeaders.h"
#include "LinesCounter.h"
#include "Utilities.h"






int32 countPagesInSpread(UIDRef spreadRef) {
	InterfacePtr<ISpread> spread(spreadRef, UseDefaultIID());
	return spread->GetNumPages();
}

/**
 * Count pages in every spread in document and return as result 
 * vector containing last page indices of every spread.
 */
std::vector<int32> getSpreadPagesIntervalsInDocument(IDocument* doc) {
	std::vector<int32> spreadPagesIntervals;
	int32 intermediatePageIndex = 0;

	InterfacePtr<IDocument>document(doc, UseDefaultIID());
	IDataBase* db = document->GetDocWorkSpace().GetDataBase();
	InterfacePtr<ISpreadList>spreadList(document, UseDefaultIID());

	for (int32 spreadIndex = 0; spreadIndex < spreadList->GetSpreadCount(); spreadIndex++) {
		intermediatePageIndex += countPagesInSpread(UIDRef(db, spreadList->GetNthSpreadUID(spreadIndex)));
		spreadPagesIntervals.push_back(intermediatePageIndex);
	}
	return spreadPagesIntervals;
}

/**
 * Count text lines in text item via it's UIDRef.
 * Return -1 if UIDRef in invalid,
 *			 if UIDRef is not reference to text object.
 */
int32 countLinesInItem(UIDRef itemRef) {
	ASSERT(itemRef);
	if (itemRef == kInvalidUIDRef) {
		return -1;
	}
	int32 linesCount = 0;
	InterfacePtr<IGraphicFrameData> graphicFrameData(itemRef, UseDefaultIID());
	if (!graphicFrameData) { return -1; }
	InterfacePtr<IMultiColumnTextFrame> multiColumnTextFrame(graphicFrameData->QueryMCTextFrame());
	if (!multiColumnTextFrame) { return -1; }
	InterfacePtr<ITextModel> textModel(multiColumnTextFrame->QueryTextModel());
	if (!textModel) { return -1; }

	InterfacePtr<IWaxStrand> waxStrand((IWaxStrand*)textModel->QueryStrand(kFrameListBoss, IID_IWAXSTRAND));
	K2::scoped_ptr<IWaxIterator> waxIterator(waxStrand->NewWaxIterator());

	IWaxLine* waxLine;

	waxLine = waxIterator->GetFirstWaxLine(0);
	//	skip lines with single system character
	if (waxLine->GetTextSpan() > 1) {
		linesCount++;
	}
	while ((waxLine = waxIterator->GetNextWaxLine()) != nil) {
		//	skip lines with single system character
		if (waxLine->GetTextSpan() > 1) {
			linesCount++;
		}
	}
	//	fix for tables
	if (textModel->TotalLength() != textModel->GetPrimaryStoryThreadSpan()) {
		linesCount--;
	}
	return linesCount;
}

/**
* Parse string representation of pageItems (like pages or pages intervals) into exact integer page numbers, saves error message if some errors were ocured.
*
* @param	pageItems		vector with string values of pageItems (like pages or pages intervals)
* @param	pageNumbers		vector for output pageItems elements parsed into page numbers
* @param	docTotalPages	total number of pages in document
* @param	errorMessage	string representing error occured during parsing, left empty if no errors were occured
* @return					kSuccess if there is no validation errors in PageItems, otherwise return kFailture
*/
ErrorCode parsePageIntervalsFromStringToIntValues(std::vector<std::string>& pageItems, std::vector<int32>& pageNumbers, const int32 docTotalPages, WideString& errorMessage) {
	for (auto it = pageItems.begin(); it != pageItems.end(); it++) {
		std::string pageItem = *it;

		if (isNumber(pageItem)) {
			int32 pageIndex = std::stoi(pageItem);
			if (!isPageValid(pageIndex, docTotalPages)) {
				errorMessage = (WideString)"Validation Error. Page index out of range.";
				return kFailure;
			}
			pageNumbers.push_back(pageIndex);
		}
		else if (isInterval(pageItem)) {
			int32 dashIndex = pageItem.find('-');
			int32 leftBorder = std::stoi(pageItem.substr(0, dashIndex));
			int32 rightBorder = std::stoi(pageItem.substr(dashIndex + 1, pageItem.size()));

			if (!isPageIntervalValid(leftBorder, rightBorder, docTotalPages)) {
				errorMessage = (WideString)"Validation Error. Interval error.";
				return kFailure;
			}

			for (int i = leftBorder; i <= rightBorder; i++) {
				pageNumbers.push_back(i);
			}
		}
		else {
			errorMessage = (WideString)"Validation Error. Only integers and symbols - , allowed.";
			return kFailure;
		}
	}
	return kSuccess;
}

std::vector<PageInDoc> getPageInDocFromAbsIndices(std::vector<int32>absPageIndexes, std::vector<int32> spreadPagesIntervals) {
	std::vector<PageInDoc>pagesInDoc;
	std::set<int32> pagesSet;
	std::copy(pagesSet.begin(), pagesSet.end(), back_inserter(absPageIndexes));

	for (auto it = pagesSet.begin(); it != pagesSet.end(); it++) {
		pagesInDoc.push_back(PageInDoc(*it, spreadPagesIntervals));
	}
	return pagesInDoc;
}




int32 countLinesOnDocumentPage(IDocument* doc, PageInDoc page) {
	int32 linesCount = 0;
	InterfacePtr<IDocument>document(doc, UseDefaultIID());
	IDataBase* db = document->GetDocWorkSpace().GetDataBase();
	InterfacePtr<ISpreadList>spreadList(document, UseDefaultIID());
	InterfacePtr<ISpread>spread(db, spreadList->GetNthSpreadUID(page.getSpreadIndex()), UseDefaultIID());
	UIDList pageItems(db);
	spread->GetItemsOnPage(page.getInSpreadIndex(), &pageItems, kFalse);

	for (int32 i; i < pageItems.Length(); i++) {
		linesCount += countLinesInItem(pageItems.GetRef(i));
	}
	return linesCount;
}


int32 countLinesInFullDocument(IDocument* doc) {
	int32 linesCount = 0;
	InterfacePtr<IDocument>document(doc, UseDefaultIID());
	IDataBase* db = document->GetDocWorkSpace().GetDataBase();
	InterfacePtr<ISpreadList>spreadList(document, UseDefaultIID());

	for (int32 spreadIndex = 0; spreadIndex < spreadList->GetSpreadCount(); spreadIndex++) {
		InterfacePtr<ISpread>spread(db, spreadList->GetNthSpreadUID(spreadIndex), UseDefaultIID());

		for (int32 pageIndex = 0; pageIndex < spread->GetNumPages(); pageIndex++) {
			UIDList pageItems(db);
			spread->GetItemsOnPage(pageIndex, &pageItems, kFalse);

			for (int32 i; i < pageItems.Length(); i++) {
				linesCount += countLinesInItem(pageItems.GetRef(i));
			}
		}
	}
	return linesCount;
}


int32 countLinesInDocumentByPages(IDocument* doc, std::vector<int32>pagesIndices) {
	if (pagesIndices.empty()) {
		return countLinesInFullDocument(doc);
	}

	int32 linesCount = 0;
	std::vector<PageInDoc>pagesInDoc = getPageInDocFromAbsIndices(pagesIndices, getSpreadPagesIntervalsInDocument(doc));

	for (int i = 0; i < pagesInDoc.size(); i++) {
		linesCount += countLinesOnDocumentPage(doc, pagesInDoc[i]);
	}
	return linesCount;
}

//int32 countLinesInDocumentByPages1(IDocument* doc, std::vector<PageInDoc>pagesInDoc) {
//	if (pagesInDoc.empty()) {
//		return countLinesInFullDocument(doc);
//	}
//
//	int32 linesCount = 0;
//	for (int i = 0; i < pagesInDoc.size(); i++) {
//		linesCount += countLinesOnDocumentPage(doc, pagesInDoc[i]);
//	}
//	return linesCount;
//}
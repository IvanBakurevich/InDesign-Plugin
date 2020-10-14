#ifndef LINESCOUNTER_H_
#define LINESCOUNTER_H_

#include "PageInDoc.h"
#include <vector>
#include <set>
#include <IDocument.h>
#include <ISpreadList.h>
#include <ISpread.h>
#include <UIDList.h>
#include <IGraphicFrameData.h>
#include <IMultiColumnTextFrame.h>
#include <ITextModel.h>
#include <IWaxStrand.h>
#include <IWaxIterator.h>
#include <IWaxLine.h>

/**
* Count number of text lines in document.
*
* @param	doc		document to count in
* @return			number of text lines
*/
int32 countLinesInFullDocument(IDocument* doc);

/**
* Count number of text lines in document on particulat pages.
*
* @param	doc				document to count in
* @param	pagesIndices	vector of pages in document in which to count (page index starts from 1)
* @return					number of text lines
*/
int32 countLinesInDocumentByPages(IDocument* doc, std::vector<int32>pagesIndices);

/**
* Parse string representation of pageItems (like pages or pages intervals) into exact integer page numbers, saves error message if some errors were ocured.
*
* @param	pageItems		vector with string values of pageItems (like pages or pages intervals)
* @param	pageNumbers		vector for output pageItems elements parsed into page numbers
* @param	docTotalPages	total number of pages in document
* @param	errorMessage	string representing error occured during parsing, left empty if no errors were occured
* @return					kSuccess if there is no validation errors in PageItems, otherwise return kFailture
*/
ErrorCode parsePageIntervalsFromStringToIntValues(std::vector<std::string>& pageItems, std::vector<int32>& pageNumbers, const int32 docTotalPages, WideString& errorMessage);

#endif

//========================================================================================
//  
//  $File: $
//  
//  Owner: Ivan Bakuvevich
//  
//  $Author: $
//  
//  $DateTime: $
//  
//  $Revision: $
//  
//  $Change: $
//  
//  Copyright 1997-2012 Adobe Systems Incorporated. All rights reserved.
//  
//  NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance 
//  with the terms of the Adobe license agreement accompanying it.  If you have received
//  this file from a source other than Adobe, then your use, modification, or 
//  distribution of it requires the prior written permission of Adobe.
//  
//========================================================================================

#include "VCPlugInHeaders.h"

// Interface includes:
#include "IActiveContext.h"
// General includes:
#include "CDialogController.h"
// Project includes:
#include "LCID.h"
#include "ISelectionManager.h"
#include "ITextEditSuite.h"
#include <ILayoutUIUtils.h>
#include <IPageList.h>
#include <IStoryList.h>
#include <IWaxIterator.h>
#include <IWaxStrand.h>
#include <IWaxLine.h>
#include <CAlert.h>
#include <IDocument.h>
#include <ISpread.h>
#include <ISpreadList.h>
#include <IGraphicFrameData.h>

#include "Utilities.h";
#include "ILCData.h";

/** LCDialogController
	Methods allow for the initialization, validation, and application of dialog widget
	values.
	Implements IDialogController based on the partial implementation CDialogController.

	
	@ingroup linescounter
*/
class LCDialogController : public CDialogController
{
private:
	std::vector<std::string> getPageItems(std::string initStr) {
		std::vector<std::string> vector;

		initStr.erase(std::remove(initStr.begin(), initStr.end(), ' '), initStr.end());

		std::stringstream ss(initStr);
		std::string itemStr;
		while (getline(ss, itemStr, ',')) {
			vector.push_back(itemStr);
		}
		return vector;
	}

	std::vector<int32> getPageNumbersFromPageItems(std::vector<std::string>& pageItems) {
		std::vector<int32> pageNumbers;

		for (auto it = pageItems.begin(); it != pageItems.end(); it++) {
			std::string pageItem = *it;

			if (isNumber(pageItem)) {
				int32 pageIndex = std::stoi(pageItem);
				pageNumbers.push_back(pageIndex);
			}
			else if (isInterval(pageItem)) {
				int32 dashIndex = pageItem.find('-');
				int32 leftBorder = std::stoi(pageItem.substr(0, dashIndex));
				int32 rightBorder = std::stoi(pageItem.substr(dashIndex + 1, pageItem.size()));

				for (int i = leftBorder; i <= rightBorder; i++) {
					pageNumbers.push_back(i);
				}
			}
		}
		return pageNumbers;
	}

	WideString pageItemsValidationError(std::vector<std::string>& pageItems, int32 docTotalPages) {
		for (auto it = pageItems.begin(); it != pageItems.end(); it++) {
			std::string pageItem = *it;

			if (isNumber(pageItem)) {
				int32 pageIndex = std::stoi(pageItem);
				if (!isPageValid(pageIndex, docTotalPages)) {
					return (WideString)"Validation Error. Page index out of range.";
				}
			}
			else if (isInterval(pageItem)) {
				int32 dashIndex = pageItem.find('-');
				int32 leftBorder = std::stoi(pageItem.substr(0, dashIndex));
				int32 rightBorder = std::stoi(pageItem.substr(dashIndex + 1, pageItem.size()));

				if (!isPageIntervalValid(leftBorder, rightBorder, docTotalPages)) {
					return (WideString)"Validation Error. Interval error.";
				}
			}
			else {
				return (WideString)"Validation Error. Only integers and symbols - , allowed.";
			}
		}
		return WideString();
	}

	public:
		/** Constructor.
			@param boss interface ptr from boss object on which this interface is aggregated.
		*/
		LCDialogController(IPMUnknown* boss) : CDialogController(boss) {}

		/** Destructor.
		*/
		virtual ~LCDialogController() {}

		/** Initialize each widget in the dialog with its default value.
			Called when the dialog is opened.
		*/
	       virtual void InitializeDialogFields(IActiveContext* dlgContext);

		/** Validate the values in the widgets.
			By default, the widget with ID kOKButtonWidgetID causes
			ValidateFields to be called. When all widgets are valid,
			ApplyFields will be called.
			@return kDefaultWidgetId if all widget values are valid, WidgetID of the widget to select otherwise.

		*/
	       virtual WidgetID ValidateDialogFields(IActiveContext* myContext);


		/** Retrieve the values from the widgets and act on them.
			@param widgetId identifies the widget on which to act.
		*/
		virtual void ApplyDialogFields(IActiveContext* myContext, const WidgetID& widgetId);
};

CREATE_PMINTERFACE(LCDialogController, kLCDialogControllerImpl)


/* ApplyFields
*/
void LCDialogController::InitializeDialogFields(IActiveContext* dlgContext)
{
	CDialogController::InitializeDialogFields(dlgContext);

	IDocument* currentDocument = dlgContext->GetContextDocument();
	if (currentDocument) {
		InterfacePtr<ILCData>data(currentDocument->GetDocWorkSpace(), UseDefaultIID());
		//	set init inputBox text
		this->SetTextControlData(kLCTextEditBoxWidgetID, data->Get());
	}
	
}

/* ValidateFields
*/
WidgetID LCDialogController::ValidateDialogFields(IActiveContext* myContext)
{
	WidgetID result = CDialogController::ValidateDialogFields(myContext);
	std::vector<std::string> pageItems;
	WideString errorMessage;

	PMString inputBoxPMString = this->GetTextControlData(kLCTextEditBoxWidgetID);
	//	empty string don't need validation.
	if (inputBoxPMString.IsEmpty()) {
		return result;
	}

	pageItems = getPageItems(inputBoxPMString.GetUTF8String());

	IDocument* currentDocument = myContext->GetContextDocument();
	//	can't apply validation without active document.
	if (currentDocument == nil) {
		return result;
	}
	InterfacePtr<IPageList> pageList(currentDocument, UseDefaultIID());

	errorMessage = pageItemsValidationError(pageItems, pageList->GetPageCount());

	if (!errorMessage.empty()) {
		result = kLCTextEditBoxWidgetID;
		CAlert::ErrorAlert(errorMessage);
	}

	return result;
}



/* ApplyFields
*/
void LCDialogController::ApplyDialogFields(IActiveContext* myContext, const WidgetID& widgetId)
{
	int32 finalLinesCount = 0;
	int32 pageLinesCount = 0;
	int32 itemLinesCount = 0;
	bool8 isCountInFullDocument;
	std::vector<std::string> pageItems;
	//	page indices starts from 1.
	std::vector<int32> pageNumbers;

	WideString output;
	WideString noActiveDocumentErrorMessage((WideString)"LC plugin: No active document found! Please, open existing one.");

	IDocument* currentDocument = myContext->GetContextDocument();
	if (currentDocument == nil) {
		//	do nothing without active document.
		CAlert::WarningAlert(noActiveDocumentErrorMessage);
		return;
	}

	PMString inputBoxPMString = this->GetTextControlData(kLCTextEditBoxWidgetID);

	// count in full documnet if empty.
	isCountInFullDocument = inputBoxPMString.IsEmpty();

	if (!isCountInFullDocument) {
		pageItems = getPageItems(inputBoxPMString.GetUTF8String());
		pageNumbers = getPageNumbersFromPageItems(pageItems);
	}


	IDataBase* db = currentDocument->GetDocWorkSpace().GetDataBase();
	InterfacePtr<ISpreadList> spreadList(currentDocument, UseDefaultIID());

	InterfacePtr<ILCData>data(currentDocument->GetDocWorkSpace(), UseDefaultIID());
	data->Set((WideString)inputBoxPMString);

	int32 documentPageIndex = 0;


	for (int32 spreadIndex = 0; spreadIndex < spreadList->GetSpreadCount(); spreadIndex++) {

		UIDRef spreadUIDRef(db, spreadList->GetNthSpreadUID(spreadIndex));
		InterfacePtr<ISpread> spread(spreadUIDRef, UseDefaultIID());


		for (int32 pageIndex = 0; pageIndex < spread->GetNumPages(); pageIndex++) {
			documentPageIndex++;

			if (!isCountInFullDocument &&
				//	skip if no such value in vector
				std::find(pageNumbers.begin(), pageNumbers.end(), documentPageIndex) == pageNumbers.end()) {
				continue;
			}

			UIDList pageItems(db);
			pageLinesCount = 0;

			spread->GetItemsOnPage(pageIndex, &pageItems, kFalse);


			for (int32 i = 0; i < pageItems.Length(); i++) {
				UIDRef pageItemRef = pageItems.GetRef(i);
				itemLinesCount = 0;

				//InterfacePtr<ITextModel> textModel(pageItemRef, UseDefaultIID());
				InterfacePtr<IGraphicFrameData> graphicFrameData(pageItemRef, UseDefaultIID());
				if (!graphicFrameData) { continue; }

				InterfacePtr<IMultiColumnTextFrame> multiColumnTextFrame(graphicFrameData->QueryMCTextFrame());
				if (!multiColumnTextFrame) { continue; }
				InterfacePtr<ITextModel> textModel(multiColumnTextFrame->QueryTextModel());

				if (textModel) {
					InterfacePtr<IWaxStrand> waxStrand((IWaxStrand*)textModel->QueryStrand(kFrameListBoss, IID_IWAXSTRAND));
					K2::scoped_ptr<IWaxIterator> waxIterator(waxStrand->NewWaxIterator());


					IWaxLine* firstLine;
					IWaxLine* nextLine;

					firstLine = waxIterator->GetFirstWaxLine(0);
					if (firstLine->GetTextSpan() > 1) {
						itemLinesCount++;
					}

					while ((nextLine = waxIterator->GetNextWaxLine()) != nil) {

						if (nextLine->GetTextSpan() > 1) {
							itemLinesCount++;
						}
					}

					if (textModel->TotalLength() != textModel->GetPrimaryStoryThreadSpan()) {
						itemLinesCount--;
					}

				}
				pageLinesCount += itemLinesCount;
			}
			finalLinesCount += pageLinesCount;
		}
	}


	output.Append((WideString)"LC plugin: ");
	output.Append((WideString)"total lines in document - ");
	output.Append((WideString)std::to_string(finalLinesCount).c_str());

	CAlert::InformationAlert(output);
}

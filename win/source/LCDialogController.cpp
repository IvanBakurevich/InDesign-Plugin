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
#include <ITextModel.h>
#include <IComposeScanner.h>
#include <IAutoFlowCmdData.h>
#include <IDataStringUtils.h>
#include <ITextStoryThreadDict.h>
#include <IWaxIterator.h>
#include <IWaxStrand.h>
#include <IWaxLine.h>
#include <CAlert.h>
#include <IDocument.h>
#include <IDataBase.h>
#include <ISpread.h>
#include <ISpreadList.h>
#include <IGraphicFrameData.h>

/** LCDialogController
	Methods allow for the initialization, validation, and application of dialog widget
	values.
	Implements IDialogController based on the partial implementation CDialogController.

	
	@ingroup linescounter
*/
class LCDialogController : public CDialogController
{
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
	// Put code to initialize widget values here.
}

/* ValidateFields
*/
WidgetID LCDialogController::ValidateDialogFields(IActiveContext* myContext)
{
	WidgetID result = CDialogController::ValidateDialogFields(myContext);

	PMString pagesInputBoxString = this->GetTextControlData(kLCTextEditBoxWidgetID);
	//	empty string don't need validation.
	if (pagesInputBoxString.IsEmpty()) {
		return result;
	}

	IDocument* currentDocument = myContext->GetContextDocument();
	//	can't apply validation without active document.
	if (currentDocument == nil) {
		return result;
	}

	InterfacePtr<IPageList> pageList((IPMUnknown*)currentDocument, IID_IPAGELIST);
	int32 docPagesCount = pageList->GetPageCount();



	std::string initStr = pagesInputBoxString.GetUTF8String();
	std::vector<std::string> vectorStr;
	std::vector<int32> vectorInt;

	initStr.erase(std::remove(initStr.begin(), initStr.end(), ' '), initStr.end());

	std::stringstream ss(initStr);
	std::string numStr;
	while (getline(ss, numStr, ',')) {
		vectorStr.push_back(numStr);
	}




	for (auto it = vectorStr.begin(); it != vectorStr.end(); it++) {
		std::string currStr = *it;
		bool isDigit = true;
		bool isInterval = true;
		for (int i = 0; i < currStr.size(); i++) {
			if (currStr[i] < '0' || currStr[i] > '9') {
				isDigit = false;
				if (currStr[i] != '-') {
					isInterval = false;
				}
			}
		}

		if (isDigit) {
			int32 intValue = std::stoi(currStr);
			if (intValue<0 || intValue> docPagesCount) {
				result = kLCTextEditBoxWidgetID;
				CAlert::InformationAlert("Value is out of range.");
			}
			vectorInt.push_back(intValue);
		}
		else if (isInterval) {
			int32 leftBorder;
			int32 rightBorder;
			size_t found;
			if ((found = currStr.find('-')) != std::string::npos) {
				leftBorder = std::stoi(currStr.substr(0, found));
				rightBorder = std::stoi(currStr.substr(found + 1, std::string::npos));
			}

			if (leftBorder >= rightBorder) {
				result = kLCTextEditBoxWidgetID;
				CAlert::InformationAlert("Invalid interval.");
			}

			if (leftBorder<0 || leftBorder > docPagesCount ||
				rightBorder<0 || rightBorder > docPagesCount) {
				result = kLCTextEditBoxWidgetID;
				CAlert::InformationAlert("Value is out of range.");
			}

			for (int i = leftBorder; i <= rightBorder; i++) {
				vectorInt.push_back(i);
			}
		}
		else {
			result = kLCTextEditBoxWidgetID;
			CAlert::InformationAlert("Only integers and symbols - , allowed.");
		}
	}


	return result;
}

/* ApplyFields
*/
void LCDialogController::ApplyDialogFields(IActiveContext* myContext, const WidgetID& widgetId)
{
	int xcount = 0;
	int32 finalLinesCount = 0;
	int32 pageLinesCount = 0;
	int32 itemLinesCount = 0;
	WideString output;
	WideString noActiveDocumentErrorMessage((WideString)"LC plugin: No active document found! Please, open existing one.");

	IDocument* currentDocument = myContext->GetContextDocument();
	if (currentDocument == nil) {
		CAlert::WarningAlert(noActiveDocumentErrorMessage);
		return;
	}



	bool8 isCountInFullDocument = kFalse;
	PMString pagesInputBoxString = this->GetTextControlData(kLCTextEditBoxWidgetID);

	//	page indices starts from 1.
	std::vector<int32> pagesIndicesVector;

	if (pagesInputBoxString.IsEmpty()) {
		isCountInFullDocument = kTrue;
	}
	else {


		std::string initStr = pagesInputBoxString.GetUTF8String();
		std::vector<std::string> vectorStr;



		initStr.erase(std::remove(initStr.begin(), initStr.end(), ' '), initStr.end());

		std::stringstream ss(initStr);
		std::string numStr;
		while (getline(ss, numStr, ',')) {
			vectorStr.push_back(numStr);
		}




		for (auto it = vectorStr.begin(); it != vectorStr.end(); it++) {
			std::string currStr = *it;
			bool isDigit = true;
			bool isInterval = true;
			for (int i = 0; i < currStr.size(); i++) {
				if (currStr[i] < '0' || currStr[i] > '9') {
					isDigit = false;
					if (currStr[i] != '-') {
						isInterval = false;
					}
				}
			}

			if (isDigit) {
				int32 intValue = std::stoi(currStr);
				pagesIndicesVector.push_back(intValue);
			}
			else if (isInterval) {
				int32 leftBorder;
				int32 rightBorder;
				size_t found;
				if ((found = currStr.find('-')) != std::string::npos) {
					leftBorder = std::stoi(currStr.substr(0, found));
					rightBorder = std::stoi(currStr.substr(found + 1, std::string::npos));
				}

				for (int i = leftBorder; i <= rightBorder; i++) {
					pagesIndicesVector.push_back(i);
				}
			}
		}


	}









	IDataBase* db = currentDocument->GetDocWorkSpace().GetDataBase();
	InterfacePtr<ISpreadList> spreadList(currentDocument, UseDefaultIID());

	int32 documentPageIndex = 0;


	for (int32 spreadIndex = 0; spreadIndex < spreadList->GetSpreadCount(); spreadIndex++) {

		UIDRef spreadUIDRef(db, spreadList->GetNthSpreadUID(spreadIndex));
		InterfacePtr<ISpread> spread(spreadUIDRef, UseDefaultIID());


		for (int32 pageIndex = 0; pageIndex < spread->GetNumPages(); pageIndex++) {
			documentPageIndex++;

			if (isCountInFullDocument == kFalse &&
				std::find(pagesIndicesVector.begin(), pagesIndicesVector.end(),documentPageIndex) == pagesIndicesVector.end()) {
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
				if (!graphicFrameData) { break; }

				InterfacePtr<IMultiColumnTextFrame> multiColumnTextFrame(graphicFrameData->QueryMCTextFrame());
				if (!multiColumnTextFrame) { break; }
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

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

	PMString::ConversionError parseErrorCode;
	int32 pagesInputBoxInt = pagesInputBoxString.GetAsNumber(&parseErrorCode);

	if (parseErrorCode != PMString::ConversionError::kNoError) {
		result = kLCTextEditBoxWidgetID;
		CAlert::InformationAlert("Value must be an integer.");
	}
	else if (pagesInputBoxInt < 0 || pagesInputBoxInt > docPagesCount) {
		result = kLCTextEditBoxWidgetID;
		CAlert::InformationAlert("Value is out of range.");
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
	WideString output;
	WideString noActiveDocumentErrorMessage((WideString)"LC plugin: No active document found! Please, open existing one.");

	IDocument* currentDocument = myContext->GetContextDocument();
	if (currentDocument == nil) {
		CAlert::WarningAlert(noActiveDocumentErrorMessage);
		return;
	}

	bool8 isCountInFullDocument = kTrue;
	int32 pageIndexToCount;
	PMString pagesInputBoxString = this->GetTextControlData(kLCTextEditBoxWidgetID);


	if (!pagesInputBoxString.IsEmpty()) {
		isCountInFullDocument = kFalse;
		//	page index starts from 1.
		pageIndexToCount = pagesInputBoxString.GetAsNumber();
	}

	IDataBase* db = currentDocument->GetDocWorkSpace().GetDataBase();
	InterfacePtr<ISpreadList> spreadList(currentDocument, UseDefaultIID());

	int32 documentPageIndex = 0;


	for (int32 spreadIndex = 0; spreadIndex < spreadList->GetSpreadCount(); spreadIndex++) {

		UIDRef spreadUIDRef(db, spreadList->GetNthSpreadUID(spreadIndex));
		InterfacePtr<ISpread> spread(spreadUIDRef, UseDefaultIID());


		for (int32 pageIndex = 0; pageIndex < spread->GetNumPages(); pageIndex++) {
			documentPageIndex++;

			if (isCountInFullDocument == kFalse && pageIndexToCount != documentPageIndex) {
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
	if (isCountInFullDocument) {
		output.Append((WideString)"total lines in document - ");
	}
	else {
		output.Append((WideString)"total lines on page ");
		output.Append((WideString)std::to_string(pageIndexToCount).c_str());
		output.Append((WideString)" - ");
	}
	output.Append((WideString)std::to_string(finalLinesCount).c_str());

	CAlert::InformationAlert(output);
}

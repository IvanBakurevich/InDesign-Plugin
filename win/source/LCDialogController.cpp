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
#include "LinesCounter.h";
#include "ILCData.h";

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
	std::vector<std::string> stringSplitItem;
	WideString errorMessage;

	IDocument* currentDocument = myContext->GetContextDocument();
	//	can't apply validation without active document.
	if (currentDocument == nil) {
		return result;
	}

	PMString inputBoxPMString = this->GetTextControlData(kLCTextEditBoxWidgetID);
	//	empty string don't need validation.
	if (inputBoxPMString.IsEmpty()) {
		return result;
	}

	stringSplitItem = splitString(inputBoxPMString.GetUTF8String());
	InterfacePtr<IPageList> pageList(currentDocument, UseDefaultIID());
	parsePageIntervalsFromStringToIntValues(stringSplitItem, std::vector<int32>(), pageList->GetPageCount(), errorMessage);

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
	int32 linesCount = 0;
	bool8 isCountInFullDocument;
	std::vector<std::string> stringSplitItem;
	//	page indices starts from 1.
	std::vector<int32> pageIndices;

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

	if (isCountInFullDocument) {
		linesCount = countLinesInFullDocument(currentDocument);
	}
	else {
		InterfacePtr<ILCData>data(currentDocument->GetDocWorkSpace(), UseDefaultIID());
		data->Set((WideString)inputBoxPMString);
		InterfacePtr<IPageList> pageList(currentDocument, UseDefaultIID());
		stringSplitItem = splitString(inputBoxPMString.GetUTF8String());
		parsePageIntervalsFromStringToIntValues(stringSplitItem, pageIndices, pageList->GetPageCount(), WideString());
		linesCount = countLinesInDocumentByPages(currentDocument, pageIndices);
	}

	output.Append((WideString)"LC plugin: ");
	output.Append((WideString)"total lines in document - ");
	output.Append((WideString)std::to_string(linesCount).c_str());
	CAlert::InformationAlert(output);
}

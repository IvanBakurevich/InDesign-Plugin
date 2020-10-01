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
#include <ITextWalker.h>
#include <IComposeScanner.h>
#include <IAutoFlowCmdData.h>

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
	// Put code to validate widget values here.
	return result;
}

/* ApplyFields
*/
void LCDialogController::ApplyDialogFields(IActiveContext* myContext, const WidgetID& widgetId)
{
	uint32 CARRIAGE_RETURN_HEX = 0x0D;
	int finalLinesCount = 0;

	IDocument* currentDocument = myContext->GetContextDocument();
	InterfacePtr<IStoryList> storyList((IPMUnknown*)currentDocument, IID_ISTORYLIST);

	for (int32 i = 0; i < storyList->GetUserAccessibleStoryCount(); i++) {
		int storyLinesCount = 0;
		WideString wstr;

		// skip invalid UIDs
		UIDRef storyRef = storyList->GetNthUserAccessibleStoryUID(i);
		if (storyRef == kInvalidUIDRef) {
			continue;
		}

		InterfacePtr<IComposeScanner> composeScanner(storyList->GetNthUserAccessibleStoryUID(i), UseDefaultIID());
		composeScanner->CopyText(0, 1024, &wstr);

		
		for (int i = 0; i < wstr.Length(); i++) {
			if (wstr.GetChar(i) == UTF32TextChar(CARRIAGE_RETURN_HEX)) {
				storyLinesCount++;
			}
		}
		storyLinesCount--; //end last carrriage return
		finalLinesCount += storyLinesCount;
	}
	//TODO: Output finalLinesCount to widget;
}

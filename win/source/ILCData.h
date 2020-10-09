//========================================================================================
//  
//  $File: //depot/devtech/15.0/plugin/source/sdksamples/basicpersistinterface/IBPIData.h $
//  
//  Owner: Adobe Developer Technologies
//  
//  $Author: pmbuilder $
//  
//  $DateTime: 2019/10/11 10:48:01 $
//  
//  $Revision: #2 $
//  
//  $Change: 1061132 $
//  
//  Copyright 1997-2010 Adobe Systems Incorporated. All rights reserved.
//  
//  NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance 
//  with the terms of the Adobe license agreement accompanying it.  If you have received
//  this file from a source other than Adobe, then your use, modification, or 
//  distribution of it requires the prior written permission of Adobe.
//  
//========================================================================================

#ifndef __ILCData_h__
#define __ILCData_h__

class WideString;
class IPMUnknown;

/**	From SDK sample; data interface that stores a WideString value. A
	persistent implementation BPIDataPersist and a non
	persistent implementation BPIData is provided in this plug-in.

	@ingroup basicpersistinterface

*/
class ILCData : public IPMUnknown
{
public:
	enum { kDefaultIID = IID_ILCDATA };

	virtual void	Set(const WideString& value) = 0;

	virtual const WideString& Get() = 0;
};

#endif // __ILCData_h__

﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace NFSDK
{
	class NFEvent : NFIEvent
	{
		public NFEvent(int nEventID, NFDataList valueList)
		{
			mnEventID = nEventID;
            mArgValueList = valueList;
		}

		public override void RegisterCallback(NFIEvent.EventHandler handler)
		{
			mHandlerDel += handler;
		}

		public override void DoEvent(NFDataList valueList)
		{
			if (null != mHandlerDel)
			{
				//mHandlerDel(mSelf, mnEventID, mArgValueList, valueList);
				mHandlerDel(mnEventID, valueList);
			}
		}

		NFGUID mSelf;
		int mnEventID;
		NFDataList mArgValueList;
		NFIEvent.EventHandler mHandlerDel;
	}
}

﻿using System;
using System.Linq;
using System.Text;
using System.Collections;
using System.Collections.Generic;

namespace NFSDK
{
	public class NFEventModule : NFIEventModule
    {
        public override void Awake() {}
        public override void Init() {}
        public override void AfterInit() {}
        public override void Execute() { }
        public override void BeforeShut() { }
        public override void Shut() {  }

        public NFEventModule(NFIPluginManager pluginManager)
        {
            mPluginManager = pluginManager;
            mhtEvent = new Dictionary<int, NFIEvent>();
		}
  
  
        public override void RegisterCallback(int nEventID, NFIEvent.EventHandler handler)
        {
            if (!mhtEvent.ContainsKey(nEventID))
            {
				mhtEvent.Add(nEventID, new NFEvent(nEventID, new NFDataList()));
            }

			NFIEvent identEvent = (NFIEvent)mhtEvent[nEventID];
            identEvent.RegisterCallback(handler);
        }

        public override void DoEvent(int nEventID, NFDataList valueList)
        {
            if (mhtEvent.ContainsKey(nEventID))
            {
                NFIEvent identEvent = (NFIEvent)mhtEvent[nEventID];
                identEvent.DoEvent(valueList);
            }
        }

        public override void DoEvent(int nEventID)
        {
			NFDataList valueList = new NFDataList();
			if (mhtEvent.ContainsKey(nEventID))
            {
				NFIEvent identEvent = (NFIEvent)mhtEvent[nEventID];
                identEvent.DoEvent(valueList);
            }
        }

        Dictionary<int, NFIEvent> mhtEvent;
    }
}
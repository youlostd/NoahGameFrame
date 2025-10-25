﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace NFSDK
{
	class NFElement : NFIElement
	{
        public NFElement()
        {
            mxPropertyManager = new NFPropertyManager(new NFGUID());
        }

        public override NFIPropertyManager GetPropertyManager()
        {
            return mxPropertyManager;
        }


        public override Int64 QueryInt(string strName)
        {
            NFIProperty xProperty = GetPropertyManager().GetProperty(strName);
            if (null != xProperty)
            {
                return xProperty.QueryInt();
            }

            return 0;
        }

        public override double QueryFloat(string strName)
        {
            NFIProperty xProperty = GetPropertyManager().GetProperty(strName);
            if (null != xProperty)
            {
                return xProperty.QueryFloat();
            }

            return 0.0;
        }

        public override string QueryString(string strName)
        {
            NFIProperty xProperty = GetPropertyManager().GetProperty(strName);
            if (null != xProperty)
            {
                return xProperty.QueryString();
            }

            return NFDataList.NULL_STRING;
        }

        public override NFGUID QueryObject(string strName)
        {
            NFIProperty xProperty = GetPropertyManager().GetProperty(strName);
            if (null != xProperty)
            {
                return xProperty.QueryObject();
            }

            return NFDataList.NULL_OBJECT;
        }

        private NFIPropertyManager mxPropertyManager;
	}
}

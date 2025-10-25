﻿//-----------------------------------------------------------------------
// <copyright file="NFCLogicClass.cs">
//     Copyright (C) 2015-2015 lvsheng.huang <https://github.com/ketoo/NFrame>
// </copyright>
//-----------------------------------------------------------------------
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;
using System.Runtime.InteropServices;
using System.Threading;
using System.IO;

namespace NFSDK
{
    public class NFClass : NFIClass
	{
        public NFClass()
        {
            mbEncrypt = false;
        }

        public override NFIPropertyManager GetPropertyManager()
        {
            return mxPropertyManager;
        }

        public override NFIRecordManager GetRecordManager()
        {
            return mxRecordManager;
        }

        public override List<string> GetConfigNameList()
        {
            return mxInstanceList;
        }

        public override List<string> GetIncludeFileList()
        {
            return mxIncludeFileList;
        }

        public override bool AddConfigName(string strConfigName)
        {
            mxInstanceList.Add(strConfigName);

            return true;
        }

        public override bool AddIncludeFile(string fileName)
        {
            mxIncludeFileList.Add(fileName);

            return true;
        }

        public override string GetName()
        {
            return mstrName;
        }

        public override void SetName(string strConfigName)
        {
            mstrName = strConfigName;
        }

        public override string GetPath()
        {
            return mstrPath;
        }

        public override void SetPath(string sPath)
        {
            mstrPath = sPath;
        }

        public override string GetInstance()
        {
            return mstrInstance;
        }

        public override void SetInstance(string strInstancePath)
        {
            mstrInstance = strInstancePath;
        }

        public override void SetEncrypt(bool bEncrypt)
        {
            mbEncrypt = bEncrypt;
        }
        public override bool GetEncrypt()
        {
            return mbEncrypt;
        }

        /////////////////////////////////////
        private NFIPropertyManager mxPropertyManager = new NFPropertyManager(new NFGUID());
        private NFIRecordManager mxRecordManager = new NFRecordManager(new NFGUID());
        private List<string> mxInstanceList = new List<string>();
        private List<string> mxIncludeFileList = new List<string>();
        private string mstrName;
        private string mstrPath;
        private string mstrInstance;
        private bool mbEncrypt;
    }
}

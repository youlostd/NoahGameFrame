﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Xml;
using System.Collections;
using System.IO;
using UnityEngine;
using System.Globalization;

namespace NFSDK
{
    public class NFElementModule : NFIElementModule
    {

        public NFElementModule(NFIPluginManager pluginManager)
        {
            mPluginManager = pluginManager;
            mhtObject = new Dictionary<string, NFIElement>();
        }

        public override void Awake() 
		{  
		}

        public override void Init()
        {
            mxLogicClassModule = mPluginManager.FindModule<NFIClassModule>();
            mstrRootPath = mxLogicClassModule.GetDataPath();
        }

        public override void AfterInit()
        {
            Load();
        }

        public override void BeforeShut()
        {
        }

        public override void Shut()
        {
        }

        public override void Execute()
        {
        }

        public override bool Load()
        {
            ClearInstanceElement();

            Dictionary<string, NFIClass> xTable = mxLogicClassModule.GetElementList();
            foreach (KeyValuePair<string, NFIClass> kv in xTable)
            {
                LoadInstanceElement(kv.Value);
            }

            return false;
        }

        public override bool Clear()
        {
            return false;
        }

        public override bool ExistElement(string strConfigName)
        {
            if (mhtObject.ContainsKey(strConfigName))
            {
                return true;
            }

            return false;
        }

        public override Int64 QueryPropertyInt(string strConfigName, string strPropertyName)
        {
            NFIElement xElement = GetElement(strConfigName);
            if (null != xElement)
            {
                return xElement.QueryInt(strPropertyName);
            }

            UnityEngine.Debug.LogError("ERROR: " + strConfigName + " HAS NO " + strPropertyName);

            return 0;
        }

        public override double QueryPropertyFloat(string strConfigName, string strPropertyName)
        {
            NFIElement xElement = GetElement(strConfigName);
            if (null != xElement)
            {
                return xElement.QueryFloat(strPropertyName);
            }

            UnityEngine.Debug.LogError("ERROR: " + strConfigName + " HAS NO " + strPropertyName);

            return 0.0;
        }

        public override string QueryPropertyString(string strConfigName, string strPropertyName)
        {
            NFIElement xElement = GetElement(strConfigName);
            if (null != xElement)
            {
                return xElement.QueryString(strPropertyName);
            }

            UnityEngine.Debug.LogError("ERROR: " + strConfigName + " HAS NO " + strPropertyName);

            return NFDataList.NULL_STRING;
        }

        public override bool AddElement(string strName, NFIElement xElement)
        {
            if (!mhtObject.ContainsKey(strName))
            {
                mhtObject.Add(strName, xElement);

                return true;
            }

            return false;
        }

        public override NFIElement GetElement(string strConfigName)
        {
            if (mhtObject.ContainsKey(strConfigName))
            {
                return (NFIElement)mhtObject[strConfigName];
            }

            return null;
        }

        private void ClearInstanceElement()
        {
            mhtObject.Clear();
        }

        private void LoadInstanceElement(NFIClass xLogicClass)
        {
            string strLogicPath = mstrRootPath;
            strLogicPath += xLogicClass.GetInstance();

            strLogicPath = strLogicPath.Replace(".xml", "");

            TextAsset textAsset = (TextAsset)Resources.Load(strLogicPath);

            XmlDocument xmldoc = new XmlDocument();
            xmldoc.LoadXml(textAsset.text);
            XmlNode xRoot = xmldoc.SelectSingleNode("XML");

            XmlNodeList xNodeList = xRoot.SelectNodes("Object");
            for (int i = 0; i < xNodeList.Count; ++i)
            {
                //NFCLog.Instance.Log("Class:" + xLogicClass.GetName());

                XmlNode xNodeClass = xNodeList.Item(i);
                XmlAttribute strID = xNodeClass.Attributes["Id"];

                //NFCLog.Instance.Log("ClassID:" + strID.Value);

                NFIElement xElement = GetElement(strID.Value);
                if (null == xElement)
                {
                    xElement = new NFElement();
                    AddElement(strID.Value, xElement);
                    xLogicClass.AddConfigName(strID.Value);

                    XmlAttributeCollection xCollection = xNodeClass.Attributes;
                    for (int j = 0; j < xCollection.Count; ++j)
                    {
                        XmlAttribute xAttribute = xCollection[j];
                        NFIProperty xProperty = xLogicClass.GetPropertyManager().GetProperty(xAttribute.Name);
                        if (null != xProperty)
                        {
                            NFDataList.VARIANT_TYPE eType = xProperty.GetType();
                            switch (eType)
                            {
                                case NFDataList.VARIANT_TYPE.VTYPE_INT:
                                    {
                                        try
                                        {
                                            NFDataList.TData xValue = new NFDataList.TData(NFDataList.VARIANT_TYPE.VTYPE_INT);
                                            xValue.Set(int.Parse(xAttribute.Value));
                                            NFIProperty property = xElement.GetPropertyManager().AddProperty(xAttribute.Name, xValue);
                                            property.SetUpload(xProperty.GetUpload());
                                        }
                                        catch
                                        {
                                            Debug.LogError("ID:" + strID.Value + " Property Name:" + xAttribute.Name + " Value:" + xAttribute.Value);
                                        }
                                    }
                                    break;
                                case NFDataList.VARIANT_TYPE.VTYPE_FLOAT:
                                    {
                                        try
                                        {
                                            NFDataList.TData xValue = new NFDataList.TData(NFDataList.VARIANT_TYPE.VTYPE_FLOAT);
                                            xValue.Set(float.Parse(xAttribute.Value, CultureInfo.InvariantCulture.NumberFormat));
                                            NFIProperty property = xElement.GetPropertyManager().AddProperty(xAttribute.Name, xValue);
                                            property.SetUpload(xProperty.GetUpload());
                                        }
                                        catch
                                        {

                                            Debug.LogError("ID:" + strID.Value + " Property Name:" + xAttribute.Name + " Value:" + xAttribute.Value);
                                        }
                                    }
                                    break;
                                case NFDataList.VARIANT_TYPE.VTYPE_STRING:
                                    {
                                        NFDataList.TData xValue = new NFDataList.TData(NFDataList.VARIANT_TYPE.VTYPE_STRING);
                                        xValue.Set(xAttribute.Value);
                                        NFIProperty property = xElement.GetPropertyManager().AddProperty(xAttribute.Name, xValue);
                                        property.SetUpload(xProperty.GetUpload());
                                    }
                                    break;
                                case NFDataList.VARIANT_TYPE.VTYPE_OBJECT:
                                    {
                                        try
                                        {
                                            NFDataList.TData xValue = new NFDataList.TData(NFDataList.VARIANT_TYPE.VTYPE_OBJECT);
                                            xValue.Set(new NFGUID(0, int.Parse(xAttribute.Value)));
                                            NFIProperty property = xElement.GetPropertyManager().AddProperty(xAttribute.Name, xValue);
                                            property.SetUpload(xProperty.GetUpload());
                                        }
                                        catch
                                        {

                                            Debug.LogError("ID:" + strID.Value + " Property Name:" + xAttribute.Name + " Value:" + xAttribute.Value);
                                        }
                                    }
                                    break;
                                case NFDataList.VARIANT_TYPE.VTYPE_VECTOR2:
                                    {
                                        try
                                        {
                                            NFDataList.TData xValue = new NFDataList.TData(NFDataList.VARIANT_TYPE.VTYPE_VECTOR2);
                                            //xValue.Set(new NFGUID(0, int.Parse(xAttribute.Value)));
                                            NFIProperty property = xElement.GetPropertyManager().AddProperty(xAttribute.Name, xValue);
                                            property.SetUpload(xProperty.GetUpload());
                                        }
                                        catch
                                        {

                                            Debug.LogError("ID:" + strID.Value + " Property Name:" + xAttribute.Name + " Value:" + xAttribute.Value);
                                        }
                                    }
                                    break;
                                case NFDataList.VARIANT_TYPE.VTYPE_VECTOR3:
                                    {
                                        try
                                        {
                                            NFDataList.TData xValue = new NFDataList.TData(NFDataList.VARIANT_TYPE.VTYPE_VECTOR3);
                                            //xValue.Set(new NFGUID(0, int.Parse(xAttribute.Value)));
                                            NFIProperty property = xElement.GetPropertyManager().AddProperty(xAttribute.Name, xValue);
                                            property.SetUpload(xProperty.GetUpload());

                                        }
                                        catch
                                        {
                                            Debug.LogError("ID:" + strID.Value + " Property Name:" + xAttribute.Name + " Value:" + xAttribute.Value);
                                        }
                                    }
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                }
            }
        }

        /////////////////////////////////////////
        private NFIClassModule mxLogicClassModule;
        /////////////////////////////////////////
        private Dictionary<string, NFIElement> mhtObject;
        private string mstrRootPath;
    }
}
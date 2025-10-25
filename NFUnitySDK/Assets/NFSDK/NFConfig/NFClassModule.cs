//-----------------------------------------------------------------------
// <copyright file="NFCClassModule.cs">
//     Copyright (C) 2015-2015 lvsheng.huang <https://github.com/ketoo/NFrame>
// </copyright>
//-----------------------------------------------------------------------
using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Xml;
using UnityEngine;

namespace NFSDK
{
    public class NFClassModule : NFIClassModule
    {

		public NFClassModule(NFIPluginManager pluginManager)
        {
            mPluginManager = pluginManager;
        }

		public override void Awake() { }


        public override void Init()
        {
            Load();
        }

		public override void AfterInit() { }
        public override void Execute() { }
        public override void BeforeShut() { }
        public override void Shut() { }

        public override void SetDataPath(string strDataPath)
        {
            mstrPath = strDataPath;
        }

        public override string GetDataPath()
        {
            return mstrPath;
        }

        private bool Load()
        {
            ClearLogicClass();
            

            string strLogicPath = mstrPath + "NFDataCfg/Struct/LogicClass";
			TextAsset textAsset = (TextAsset) Resources.Load(strLogicPath); 

			XmlDocument xmldoc = new XmlDocument ();
			xmldoc.LoadXml ( textAsset.text );
            XmlNode root = xmldoc.SelectSingleNode("XML");

            LoadLogicClass(root);
            LoadLogicClassProperty();
            LoadLogicClassRecord();
            LoadLogicClassInclude();


            ProcessLogicClassIncludeFiles();

            return false;
        }

        public override bool ExistElement(string strClassName)
        {
            if (mhtObject.ContainsKey(strClassName))
            {
                return true;
            }

            return false;
        }

        public override bool AddElement(string strName)
        {
            if (!mhtObject.ContainsKey(strName))
            {
                NFIClass xElement = new NFClass();
                xElement.SetName(strName);
                xElement.SetEncrypt(false);

                mhtObject.Add(strName, xElement);

                return true;
            }

            return false;
        }

        public override NFIClass GetElement(string strClassName)
        {
            if (mhtObject.ContainsKey(strClassName))
            {
                return (NFIClass)mhtObject[strClassName];
            }

            return null;
        }
        /////////////////////////////////////////
        private void LoadLogicClass(XmlNode xNode)
        {
            XmlNodeList xNodeList = xNode.SelectNodes("Class");
            for (int i = 0; i < xNodeList.Count; ++i)
            {
                XmlNode xNodeClass = xNodeList.Item(i);
                XmlAttribute strID = xNodeClass.Attributes["Id"];
                XmlAttribute strPath = xNodeClass.Attributes["Path"];
                XmlAttribute strInstancePath = xNodeClass.Attributes["InstancePath"];

                NFIClass xLogicClass = new NFClass();
                mhtObject.Add(strID.Value, xLogicClass);

                xLogicClass.SetName(strID.Value);
                xLogicClass.SetPath(strPath.Value);
                xLogicClass.SetInstance(strInstancePath.Value);
                xLogicClass.SetEncrypt(false);

                XmlNodeList xNodeSubClassList = xNodeClass.SelectNodes("Class");
                if (xNodeSubClassList.Count > 0)
                {
                    LoadLogicClass(xNodeClass);
                }
            }
        }
        
        private void ClearLogicClass()
        {
            mhtObject.Clear();
        }

        private void LoadLogicClassProperty()
        {
            Dictionary<string, NFIClass> xTable = GetElementList();
            foreach (KeyValuePair<string, NFIClass> kv in xTable)
            {
                LoadLogicClassProperty(kv.Value, mstrPath + kv.Value.GetPath());
            }

            //再为每个类加载iobject的属性
            foreach (KeyValuePair<string, NFIClass> kv in xTable)
            {
                if (kv.Key != "IObject")
                {
                    AddBasePropertyFormOther(kv.Value, "IObject");
                }
            }
        }

        private void LoadLogicClassRecord()
        {
            Dictionary<string, NFIClass> xTable = GetElementList();
            foreach (KeyValuePair<string, NFIClass> kv in xTable)
            {
                LoadLogicClassRecord(kv.Value, mstrPath + kv.Value.GetPath());
            }
        }

        private void LoadLogicClassProperty(NFIClass xLogicClass, string strLogicPath)
        {
            if (null != xLogicClass)
            {
				strLogicPath = strLogicPath.Replace (".xml", "");

				TextAsset textAsset = (TextAsset) Resources.Load(strLogicPath); 

				XmlDocument xmldoc = new XmlDocument ();
				xmldoc.LoadXml ( textAsset.text );
				XmlNode xRoot = xmldoc.SelectSingleNode("XML");

                XmlNode xNodePropertys = xRoot.SelectSingleNode("Propertys");
                XmlNodeList xNodeList = xNodePropertys.SelectNodes("Property");
                for (int i = 0; i < xNodeList.Count; ++i)
                {
                    XmlNode xPropertyNode = xNodeList.Item(i);
                    XmlAttribute strID = xPropertyNode.Attributes["Id"];
                    XmlAttribute strType = xPropertyNode.Attributes["Type"];
                    XmlAttribute strUpload = xPropertyNode.Attributes["Upload"];
                    bool bUpload = strUpload.Value.Equals("1");

                    switch (strType.Value)
                    {
                        case "int":
                            {
                                NFDataList.TData xValue = new NFDataList.TData(NFDataList.VARIANT_TYPE.VTYPE_INT);
                                NFIProperty xProperty = xLogicClass.GetPropertyManager().AddProperty(strID.Value, xValue);
                                xProperty.SetUpload(bUpload);
                            }
                            break;
                        case "float":
                            {

                                NFDataList.TData xValue = new NFDataList.TData(NFDataList.VARIANT_TYPE.VTYPE_FLOAT);
                                NFIProperty xProperty = xLogicClass.GetPropertyManager().AddProperty(strID.Value, xValue);
                                xProperty.SetUpload(bUpload);
                            }
                            break;
                        case "string":
                            {

                                NFDataList.TData xValue = new NFDataList.TData(NFDataList.VARIANT_TYPE.VTYPE_STRING);
                                NFIProperty xProperty = xLogicClass.GetPropertyManager().AddProperty(strID.Value, xValue);
                                xProperty.SetUpload(bUpload);
                            }
                            break;
                        case "object":
                            {
                                NFDataList.TData xValue = new NFDataList.TData(NFDataList.VARIANT_TYPE.VTYPE_OBJECT);
                                NFIProperty xProperty = xLogicClass.GetPropertyManager().AddProperty(strID.Value, xValue);
                                xProperty.SetUpload(bUpload);
                            }
                            break;
                        case "vector2":
                            {
                                NFDataList.TData xValue = new NFDataList.TData(NFDataList.VARIANT_TYPE.VTYPE_VECTOR2);
                                NFIProperty xProperty = xLogicClass.GetPropertyManager().AddProperty(strID.Value, xValue);
                                xProperty.SetUpload(bUpload);
                            }
                            break;
                        case "vector3":
                            {
                                NFDataList.TData xValue = new NFDataList.TData(NFDataList.VARIANT_TYPE.VTYPE_VECTOR3);
                                NFIProperty xProperty = xLogicClass.GetPropertyManager().AddProperty(strID.Value, xValue);
                                xProperty.SetUpload(bUpload);
                            }
                            break;
                        default:
                            break;

                    }
                }
            }
        }

        private void LoadLogicClassRecord(NFIClass xLogicClass, string strLogicPath)
        {
            if (null != xLogicClass)
            {
				strLogicPath = strLogicPath.Replace (".xml", "");

				TextAsset textAsset = (TextAsset) Resources.Load(strLogicPath); 

				XmlDocument xmldoc = new XmlDocument ();
				xmldoc.LoadXml ( textAsset.text );
				XmlNode xRoot = xmldoc.SelectSingleNode("XML");

                XmlNode xNodePropertys = xRoot.SelectSingleNode("Records");
                if (null != xNodePropertys)
                {
                    XmlNodeList xNodeList = xNodePropertys.SelectNodes("Record");
                    if (null != xNodeList)
                    {
                        for (int i = 0; i < xNodeList.Count; ++i)
                        {
                            XmlNode xRecordNode = xNodeList.Item(i);

                            string strID = xRecordNode.Attributes["Id"].Value;
                            string strRow = xRecordNode.Attributes["Row"].Value;
                            string strUpload = xRecordNode.Attributes["Upload"].Value;
                            bool bUpload = strUpload.Equals("1");
							NFDataList xValue = new NFDataList();
                            NFDataList xTag = new NFDataList();

                            XmlNodeList xTagNodeList = xRecordNode.SelectNodes("Col");
                            for (int j = 0; j < xTagNodeList.Count; ++j)
                            {
                                XmlNode xColTagNode = xTagNodeList.Item(j);

                                XmlAttribute strTagID = xColTagNode.Attributes["Tag"];
                                XmlAttribute strTagType = xColTagNode.Attributes["Type"];

								xTag.AddString (strTagID.Value);

                                switch (strTagType.Value)
                                {
                                    case "int":
                                        {
                                            xValue.AddInt(0);
                                        }
                                        break;
                                    case "float":
                                        {
                                            xValue.AddFloat(0.0);
                                        }
                                        break;
                                    case "string":
                                        {
                                            xValue.AddString("");
                                        }
                                        break;
                                    case "object":
                                        {
                                            xValue.AddObject(new NFGUID(0, 0));
                                        }
                                        break;
										case "vector2":
										{
											xValue.AddVector2(NFVector2.Zero());
										}
										break;
										case "vector3":
										{
											xValue.AddVector3(NFVector3.Zero());
										}
										break;
                                    default:
                                        break;

                                }
                            }
							NFIRecord xRecord = xLogicClass.GetRecordManager().AddRecord(strID, int.Parse(strRow), xValue, xTag);
                            xRecord.SetUpload(bUpload);
                        }
                    }
                }
            }
        }

        private void LoadLogicClassInclude()
        {
            Dictionary<string, NFIClass> xTable = GetElementList();
            foreach (KeyValuePair<string, NFIClass> kv in xTable)
            {
                LoadLogicClassInclude(kv.Key);
            }

        }

        private void LoadLogicClassInclude(string strName)
        {
            NFIClass xLogicClass = GetElement(strName);
            if (null != xLogicClass)
            {
                string strLogicPath = mstrPath + xLogicClass.GetPath();

                strLogicPath = strLogicPath.Replace(".xml", "");

                TextAsset textAsset = (TextAsset)Resources.Load(strLogicPath);

                XmlDocument xmldoc = new XmlDocument();
                xmldoc.LoadXml(textAsset.text);
                XmlNode xRoot = xmldoc.SelectSingleNode("XML");

                XmlNode xNodePropertys = xRoot.SelectSingleNode("Includes");
                XmlNodeList xNodeList = xNodePropertys.SelectNodes("Include");
                for (int i = 0; i < xNodeList.Count; ++i)
                {
                    XmlNode xPropertyNode = xNodeList.Item(i);
                    XmlAttribute strID = xPropertyNode.Attributes["Id"];
                    //int start = strID.Value.LastIndexOf('/') + 1;
                    //int end = strID.Value.LastIndexOf('.');
                    //string className = strID.Value.Substring(start, end - start);

                    xLogicClass.AddIncludeFile(strID.Value);
                }
            }
        }

        void AddBasePropertyFormOther(NFIClass xLogicClass, string strOther)
        {
            NFIClass xOtherClass = GetElement(strOther);
            if (null != xLogicClass && null != xOtherClass)
            {
                NFDataList xValue = xOtherClass.GetPropertyManager().GetPropertyList();
                for (int i = 0; i < xValue.Count(); ++i)
                {
                    NFIProperty xProperty = xOtherClass.GetPropertyManager().GetProperty(xValue.StringVal(i));
                    xLogicClass.GetPropertyManager().AddProperty(xValue.StringVal(i), xProperty.GetData());
                }
            }
        }

        void AddBaseRecordFormOther(NFIClass xLogicClass, string strOther)
        {
            NFIClass xOtherClass = GetElement(strOther);
            if (null != xLogicClass && null != xOtherClass)
            {
                NFDataList xValue = xOtherClass.GetRecordManager().GetRecordList();
                for (int i = 0; i < xValue.Count(); ++i)
                {
                    NFIRecord record = xOtherClass.GetRecordManager().GetRecord(xValue.StringVal(i));

                    xLogicClass.GetRecordManager().AddRecord(xValue.StringVal(i), record.GetRows(), record.GetColsData(), record.GetTagData());
                }
            }
        }

        private void ProcessLogicClassIncludeFiles()
        {
            Dictionary<string, NFIClass> xTable = GetElementList();
            foreach (KeyValuePair<string, NFIClass> kv in xTable)
            {
                ProcessLogicClassIncludeFiles(kv.Value);
            }
        }

        private void ProcessLogicClassIncludeFiles(NFIClass classObject)
        {
            List<string> includeFiles = classObject.GetIncludeFileList();
            foreach (var item in includeFiles)
            {
                LoadLogicClassProperty(classObject, item);
                LoadLogicClassRecord(classObject, item);
            }
        }

        public override Dictionary<string, NFIClass> GetElementList()
        {
            return mhtObject;
        }
        /////////////////////////////////////////
        private Dictionary<string, NFIClass> mhtObject = new Dictionary<string, NFIClass>();
        private string mstrPath = "";
    }
}
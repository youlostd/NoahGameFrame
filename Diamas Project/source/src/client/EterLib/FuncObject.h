#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


template<typename T>
class CFuncObject
{
	public:
		CFuncObject()
		{
			Clear();
		}

		virtual ~CFuncObject()
		{
		}

		void Clear()
		{
			m_pSelfObject = NULL;
			m_pFuncObject = NULL;
		}

		void Set(T* pSelfObject, void (T::*pFuncObject)())
		{
			m_pSelfObject = pSelfObject;
			m_pFuncObject = pFuncObject;
		}

		bool IsEmpty()
		{
			if (m_pSelfObject != NULL)
				return false;

			if (m_pFuncObject != NULL)
				return false;

			return true;
		}

		void Run()
		{
			if (m_pSelfObject)
				if (m_pFuncObject)
					std::invoke(m_pFuncObject, m_pSelfObject);
		}

		template <typename ...Params>
		bool Run(Params&&... params)
		{
			if (m_pSelfObject)
				if (m_pFuncObject)
					return std::invoke(m_pFuncObject, m_pSelfObject, std::forward<Params>(params)...);

			return false;
		}
	protected:
		T *	m_pSelfObject;
		void (T::*m_pFuncObject) ();
};



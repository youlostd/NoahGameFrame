#ifndef METIN2_CLIENT_GAMELIB_FLYTARGET_HPP
#define METIN2_CLIENT_GAMELIB_FLYTARGET_HPP

#include <Config.hpp>

#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <unordered_set>



class CFlyTarget;

class IFlyTargetableObject
{
		friend class CFlyTarget;
	public:
		IFlyTargetableObject() {}
		virtual ~IFlyTargetableObject() {}

		virtual Vector3 OnGetFlyTargetPosition() = 0;
		virtual void OnShootDamage() = 0;

	protected:
		void ClearFlyTargeter();

	private:
		void AddFlyTargeter(CFlyTarget* pTargeter);
		void RemoveFlyTargeter(CFlyTarget* pTargeter);

		std::unordered_set<CFlyTarget*> m_FlyTargeterSet;
};

class CFlyTarget
{
	public:
		enum EType
		{
			TYPE_NONE,
			TYPE_OBJECT,
			TYPE_POSITION,
		};

	public:
		CFlyTarget();
		CFlyTarget(IFlyTargetableObject * pFlyTarget);
		CFlyTarget(const Vector3& v3FlyTargetPosition);
		CFlyTarget(const CFlyTarget& rhs);
		virtual ~CFlyTarget();

		void Clear();
		bool IsObject();
		bool IsPosition();
		bool IsValidTarget();
		void NotifyTargetClear();

		const Vector3 & GetFlyTargetPosition() const;
		EType GetType();

		IFlyTargetableObject * GetFlyTarget();

		CFlyTarget & operator = (const CFlyTarget & rhs);
		void GetFlyTargetData(CFlyTarget * pFlyTarget);

	private:
		void __Initialize();

	private:
		mutable Vector3 m_v3FlyTargetPosition;
		IFlyTargetableObject * m_pFlyTarget;

		EType m_eType;
};



#endif

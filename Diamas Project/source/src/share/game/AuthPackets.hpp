#ifndef METIN2_SHARE_GAME_MASTERPACKETS_HPP
#define METIN2_SHARE_GAME_MASTERPACKETS_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif

#include <game/Types.hpp>
#include <boost/fusion/include/adapt_struct.hpp>



// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// You need to update the BOOST_FUSION_ADAPT_STRUCT calls at the end of the
// file if you modify any of the packets. Otherwise everything will break!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//
// Client -> Auth
//

enum ClientToAuthPackets
{
	kCaNone,
	kCaAuth,
	kCaPinResponse,
	kCaHGuardInfo,
	kCaHGuardCode,
	kCaRegisterRequest,
	kCaRegisterConfirm,
};

struct CaAuthPacket
{
	std::string login;
	std::string password;
	std::string hwid;
	uint32_t version;
};

struct CaRegisterPacket {
    std::string login;
    std::string password;
    std::string hwid;
    std::string deleteCode;
    std::string mailAdress;
};

struct CaPinResponsePacket
{
	uint32_t code;
};

struct CaHGuardCodePacket
{
	std::string code;
};

//
// Auth -> Client
//

enum AuthToClientPackets
{
	kAcNone,
	kAcPinRequest,
	kAcSetPinRequest,
	kAcAuthFailure,
	kAcAuthSuccess,
	kAcHGuardRequest,
	kAcHGuardResult,
	kAcRegisterStatus,
	kAcRegisterResponse,
};

struct AcRequestPacket
{
	// empty
};

struct AcAuthFailurePacket
{
	std::string status;
};

struct AcHGuardResultPacket
{
	enum
	{
		kRequireCode,
		kIncorrectCode
	};
	uint8_t code;
};

struct AcAuthSuccessPacket
{
	SessionId sessionId;
};

struct CaHGuardInfoPacket
{
	std::string macAddr;
	std::string guid;
	std::string cpuId;
};

BOOST_FUSION_ADAPT_STRUCT(
	CaHGuardInfoPacket,
	macAddr,
	guid,
	cpuId
);

BOOST_FUSION_ADAPT_STRUCT(
	CaAuthPacket,
	login,
	password,
	hwid,
	version
);

BOOST_FUSION_ADAPT_STRUCT(
    CaRegisterPacket,
    login,
    password,
    hwid,
    deleteCode,
    mailAdress
);

BOOST_FUSION_ADAPT_STRUCT(
	CaPinResponsePacket,
	code
);

BOOST_FUSION_ADAPT_STRUCT(
	CaHGuardCodePacket,
	code
);

// Empty structs don't work.
//BOOST_FUSION_ADAPT_STRUCT(
//	AcRequestPacket
//);

BOOST_FUSION_ADAPT_STRUCT(
	AcHGuardResultPacket,
	code
);

BOOST_FUSION_ADAPT_STRUCT(
	AcAuthFailurePacket,
	status
);

BOOST_FUSION_ADAPT_STRUCT(
	AcAuthSuccessPacket,
	sessionId
);


#endif
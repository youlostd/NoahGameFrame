#ifndef METIN2_SERVER_MASTER_ONLINEPLAYERS_HPP
#define METIN2_SERVER_MASTER_ONLINEPLAYERS_HPP


#if VSTD_HAS_PRAGMA_ONCE
#pragma once
#endif


#include <unordered_map>
#include <memory>
#include <set>

class GameSocket;
class Server;

// Struct describing a player that currently plays on one of the connected
// game servers.
struct OnlinePlayer
{
	// TODO(tim): weak_ptr?
	GameSocket* game;
	uint32_t aid;
	uint32_t pid;
	std::string name;
	uint32_t mapIndex;
	uint8_t empire;
	uint8_t channel;
	std::string hwid;
};

template <class T>
class VectorSet
{
public:
  using iterator                     = typename std::vector<T>::iterator;
  using const_iterator               = typename std::vector<T>::const_iterator;
  iterator begin()                   { return theVector.begin(); }
  iterator end()                     { return theVector.end(); }
  const_iterator begin() const       { return theVector.begin(); }
  const_iterator end() const         { return theVector.end(); }
  const T& front() const             { return theVector.front(); }
  const T& back() const              { return theVector.back(); }
  void insert(const T& item)         { if (theSet.insert(item).second) theVector.push_back(item); }
  size_t count(const T& item) const  { return theSet.count(item); }
  bool empty() const                 { return theSet.empty(); }
  size_t size() const                { return theSet.size(); }
  void erase(const T& item) {
      theSet.erase(item);
	  theVector.erase(std::remove(theVector.begin(), theVector.end(), item), theVector.end());
  }
private:
  std::vector<T> theVector;
  std::set<T>    theSet;
};

class OnlinePlayers
{
	public:
		OnlinePlayers(Server& server);

		void Enter(OnlinePlayer player);
        void StartExpiringMultiFarm(uint32_t aid, const std::string& hwid);
        void StopExpiringMultiFarm(uint32_t aid, const std::string& hwid);
        void Update();
        void UpdateDropStatus(std::string hwid);
        void Leave(uint32_t pid);

		void RemoveAll(GameSocket* game);

		const OnlinePlayer* Get(uint32_t pid) const;
		const OnlinePlayer* Get(const std::string& name) const;
		const OnlinePlayer* GetByAid(uint32_t aid) const;

		OnlinePlayer*                                 Get(uint32_t pid);
		OnlinePlayer*                                 Get(const std::string& name);
		OnlinePlayer*                                 GetByAid(uint32_t aid);
        std::vector<std::pair<std::string, uint32_t>> GetByHwid(const std::string& hwid);

    private:
		void BroadcastEnter(const OnlinePlayer* player);
		void BroadcastLeave(const OnlinePlayer* player);

		Server& m_server;

		std::unordered_map<uint32_t, std::unique_ptr<OnlinePlayer>> m_players;

		std::unordered_map<uint32_t, OnlinePlayer*> m_playersByAid;

		// key is owned by OnlinePlayer*, which is owned by |m_players|
		std::unordered_map<std::string, OnlinePlayer*> m_playersByName;
        std::unordered_map<std::string, VectorSet<uint32_t>> m_playersByHwid;
		std::vector<std::pair<std::string, uint32_t>> m_expiringMultiFarmBlock;

};



#endif

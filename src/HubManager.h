/*
 * HubManager.h - Class for managing a system with a set of hubs
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
*/

#if defined(ESP32)

#ifndef HubManager_h
#define HubManager_h

#include "Lpf2Hub.h"

// // abstraction to lpf2hub only with the information which is relevant for the hub manager
// class ManagedHub
// {
// public:
//   ManagedHub(HubType type, std::string name = null, std::string address = null);

//   void Connect();
//   //void AddHubProperty(HubPropertyReference[] properties, HubPropertyChangeCallback hubPropertyChangeCallback = nullptr);
//   //void AddDevice(DeviceType type, byte port = null, PortValueChangeCallback valueChangeCallback = nullptr);

//   //std::vector<Device *> *GetDevices();
//   std::string GetName();
//   HubType GetType();
//   std::string GetAddress();
//   bool IsConnected();

// private:
//   bool _isConnected;
//   HubType _hubType;
//   std::string _name;
//   std::string _address;
//   Lpf2Hub _hub;
// }

// manages a set of hubs as a system which could be started and the hub manager takes care
// of connecting/reconnecting all attached hubs
class HubManager
{

public:
  HubManager();
  void AddHub(Lpf2Hub hub, std::string address, std::string name);
  std::vector<Lpf2Hub*> ManagedHubs;
  void Start();
  void Stop();
  Lpf2Hub* GetHubByAddress(NimBLEAddress address);
  Lpf2Hub* GetHubByAddress(std::string address);
  Lpf2Hub* GetHubByName(std::string name);
  //bool IsConnectionFinished();
};

#endif // HubManager_h

#endif // ESP32

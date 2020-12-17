/*
 * HubManager.cpp - Class for managing a system with a set of hubs
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
*/

#if defined(ESP32)

#ifndef HubManager_h
#define HubManager_h

#include "Lpf2Hub.h"

/**
 * @brief Constructor
 */

ManagedHub::ManagedHub(HubType type, std::string name, std::string address)
{
  _type = type;
  _name = name;
  _address = address;
};


// abstraction to lpf2hub only with the information which is relevant for the hub manager
class ManagedHub
{
public:
  ManagedHub(HubType type, std::string name = null, std::string deviceAddress = null);

  void Connect();
  void AddHubProperty(HubPropertyReference[] properties, HubPropertyChangeCallback hubPropertyChangeCallback = nullptr);
  void AddDevice(DeviceType type, byte port = null, PortValueChangeCallback valueChangeCallback = nullptr);

  std::vector<Device*>* GetDevices();
  std::string GetName();
  HubType GetType();
  std::string GetAddress();
  bool IsConnected();

  private:
   bool _isConnected;
   HubType _type;
   std::string _name;
   std::string _address;
   Lpf2Hub _hub;
}

// manages a set of hubs as a system which could be started and the hub manager takes care
// of connecting/reconnecting all attached hubs
class HubManager
{

public:
  HubManager(bool automaticReconnect = true);
  void AddHub(ManagedHub hub);
  std::vector<ManagedHub*> ManagedHubs;
  void Start();
  void Stop();
  bool IsSystemReady();
};

#endif // HubManager_h

#endif // ESP32

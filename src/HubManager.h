/*
 * HubManager.h - Class for managing different hubs and the connections
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
*/

#if defined(ESP32)

#ifndef HubManager_h
#define HubManager_h

#include "Lpf2Hub.h"



class ManagedHub
{
public:
  ManagedHub();
  ManagedHub(HubType type, std::string name = null, std::string deviceAddress = null);

  void Connect();
  void AddHubProperty(HubPropertyReference[] properties, HubPropertyChangeCallback hubPropertyChangeCallback = nullptr);
  void AddDevice(DeviceType type, byte port = null, PortValueChangeCallback valueChangeCallback = nullptr);

  std::vector<Device*>* GetDevices();
  std::string GetName();
  HubType GetType();
  std::string GetAddress();
  bool IsConnected();
}

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

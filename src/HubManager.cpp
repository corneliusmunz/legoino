/*
 * HubManager.cpp - Class for managing a system with a set of hubs
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
*/

//#if defined(ESP32)

#include "HubManager.h"

/** 
 * Callback if a scan has ended with the results of found devices 
 * only needed to enforce the non blocking scan start
 */

void HubManagerScanEndedCallback(NimBLEScanResults results)
{
  Serial.print("Number of Devices: ");
  Serial.println(results.getCount());
  log_d("Number of devices: %d", results.getCount());
  for (int i = 0; i < results.getCount(); i++)
  {
    Serial.println(results.getDevice(i).toString().c_str());
    log_d("device[%d]: %s", i, results.getDevice(i).toString().c_str());
  }
}




// ManagedHub::ManagedHub(HubType type, std::string name, std::string address)
// {
//   _type = type;
//   _name = name;
//   _address = address;
//   _hub = Lpf2Hub();
// };

// ManagedHub::Connect()
// {
//   _hub.init();
// }

class HubManagerAdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks
{
  HubManager *_hubManager;

public:
  HubManagerAdvertisedDeviceCallbacks(HubManager *hubManager) : NimBLEAdvertisedDeviceCallbacks()
  {
    _hubManager = hubManager;
  }

  void onResult(NimBLEAdvertisedDevice *advertisedDevice)
  {
    log_d("advertised device: %s", advertisedDevice->toString().c_str());
    Serial.println("Advertised device:");
    Serial.println(advertisedDevice->toString().c_str());

    //Found a device, check if the service is contained
    if (advertisedDevice->haveServiceUUID() && advertisedDevice->getServiceUUID().equals(BLEUUID(LPF2_UUID)))
    {
      Serial.println("Lego device found");
      advertisedDevice->getScan()->stop();

      // check if the device is contained in the defined hubs of the hub manager
      Serial.print("Address: ");
      Serial.println(advertisedDevice->getAddress().toString().c_str());
      Lpf2Hub *discoveredHub = _hubManager->GetHubByAddress(advertisedDevice->getAddress());
      Serial.println("found hub");
      if (discoveredHub != nullptr)
      {
        // set hub type
        if (advertisedDevice->haveManufacturerData())
        {
          uint8_t *manufacturerData = (uint8_t *)advertisedDevice->getManufacturerData().data();
          uint8_t manufacturerDataLength = advertisedDevice->getManufacturerData().length();
          if (manufacturerDataLength >= 3)
          {
            log_d("manufacturer data hub type: %x", manufacturerData[3]);
            //check device type ID
            switch (manufacturerData[3])
            {
            case DUPLO_TRAIN_HUB_ID:
              discoveredHub->_hubType = HubType::DUPLO_TRAIN_HUB;
              break;
            case BOOST_MOVE_HUB_ID:
              discoveredHub->_hubType = HubType::BOOST_MOVE_HUB;
              break;
            case POWERED_UP_HUB_ID:
              Serial.println("Found Train Hub");
              discoveredHub->_hubType = HubType::POWERED_UP_HUB;
              break;
            case POWERED_UP_REMOTE_ID:
              Serial.println("Found remote");
              discoveredHub->_hubType = HubType::POWERED_UP_REMOTE;
              break;
            case CONTROL_PLUS_HUB_ID:
              discoveredHub->_hubType = HubType::CONTROL_PLUS_HUB;
              break;
            default:
              discoveredHub->_hubType = HubType::UNKNOWNHUB;
              break;
            }
          }
        }

        // start connection of hub
        discoveredHub->_isConnecting = true;
        discoveredHub->_pServerAddress = new BLEAddress(advertisedDevice->getAddress());
        discoveredHub->_hubName = advertisedDevice->getName();
        //discoveredHub->connectHub();

        //check if all defined hubs are discovered. If yes, stop scan
        if (!_hubManager->IsDiscoveryFinished())
        {
          advertisedDevice->getScan()->start(10, HubManagerScanEndedCallback);
        } else
        {
          _hubManager->StartConnection();
        }
        
      }
    }
  }
};

// // abstraction to lpf2hub only with the information which is relevant for the hub manager
// class ManagedHub
// {
// public:
//   ManagedHub(HubType type, std::string name = null, std::string deviceAddress = null);

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
//   HubType _type;
//   std::string _name;
//   std::string _address;
//   Lpf2Hub _hub;
// }

/**
 * @brief Constructor
 */
HubManager::HubManager(){};

void HubManager::StartDiscovery(uint32_t scanDuration, bool nonBlocking)
{

  BLEDevice::init("");
  BLEScan *pBLEScan = BLEDevice::getScan();

  pBLEScan->setAdvertisedDeviceCallbacks(new HubManagerAdvertisedDeviceCallbacks(this));

  pBLEScan->setActiveScan(true);
  // start method with callback function to enforce the non blocking scan. If no callback function is used,
  // the scan starts in a blocking manner
  Serial.println("Start scan");
  if (nonBlocking)
  {
    pBLEScan->start(scanDuration, HubManagerScanEndedCallback);
  }
  else
  {
    pBLEScan->start(scanDuration);
  }
}

void HubManager::StartConnection()
{
  Serial.println("StartConnection");
  for (int i = 0; i < ManagedHubs.size(); i++)
  {
    Serial.print("Connect Hub: ");
    Serial.println(i, DEC);
    ManagedHubs[i]->connectHub();
  }
}

void HubManager::AddHub(Lpf2Hub hub, std::string address)
{
  hub._requestedDeviceAddress = new BLEAddress(address);
  //Serial.println(hub.getHubAddress().toString().c_str());
  ManagedHubs.push_back(&hub);
}

bool HubManager::IsDiscoveryFinished()
{
  for (int i = 0; i < ManagedHubs.size(); i++)
  {
    if (!(ManagedHubs[i]->isConnecting()))
    {
      return false;
    }
  }

  return true;
}

bool HubManager::IsConnectionFinished()
{
  Serial.print("HubSize: ");
  Serial.println(ManagedHubs.size(), DEC);
  for (int i = 0; i < ManagedHubs.size(); i++)
  {
    if (!ManagedHubs[i]->isConnected())
    {
      Serial.println("not connected");
      return false;
    }
  }
  Serial.println("connected");
  return true;
}

// bool NimBLEAddress::equals(const NimBLEAddress &otherAddress) const {
//     return *this == otherAddress;
// } // equals
Lpf2Hub *HubManager::GetHubByAddress(NimBLEAddress address)
{
  Serial.println("GetHubByAddress");
  for (int i = 0; i < ManagedHubs.size(); i++)
  {
    //Serial.println(ManagedHubs[i]->*_requestedDeviceAddress->toString()->c_str());
    Serial.println("inside loop");
    Serial.println(address.toString().c_str());
    return ManagedHubs[i];
    NimBLEAddress hubAddress = *(ManagedHubs[i]->_requestedDeviceAddress);
    if (address.equals(hubAddress))
    {
      Serial.println("found hub with requested address");
      return ManagedHubs[i];
    }
  }

  return nullptr;
}

//#endif // ESP32

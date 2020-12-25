/*
 * HubManager.cpp - Class for managing a system with a set of hubs
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
*/

//#if defined(ESP32)

#include "HubManager.h"

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

      // check if the device is contained in the defined hubs of the hub manager
      Serial.print("Address: ");
      Serial.println(advertisedDevice->getAddress().toString().c_str());
      Lpf2Hub* discoveredHub = _hubManager->GetHubByAddress(advertisedDevice->getAddress());
      if (discoveredHub == nullptr)
      {
        discoveredHub = _hubManager->GetHubByName(advertisedDevice->getName());
      }
      if (discoveredHub != nullptr)
      {
        Serial.println("found hub - stop scan");
        advertisedDevice->getScan()->stop();
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
              discoveredHub->_hubType = HubType::POWERED_UP_HUB;
              break;
            case POWERED_UP_REMOTE_ID:
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
        discoveredHub->_isConnecting = true;
        discoveredHub->_pServerAddress = new BLEAddress(advertisedDevice->getAddress());
        discoveredHub->_hubName = advertisedDevice->getName();
        //discoveredHub.connectHub();
      }

      //   // start connection of hub
      //   discoveredHub._isConnecting = true;
      //   discoveredHub._pServerAddress = new BLEAddress(advertisedDevice->getAddress());
      //   discoveredHub._hubName = advertisedDevice->getName();
      //   //discoveredHub.connectHub();

      //   //check if all defined hubs are discovered. If yes, stop scan
      //   if (!_hubManager->IsDiscoveryFinished())
      //   {
      //     advertisedDevice->getScan()->start(10, HubManagerScanEndedCallback);
      //   } else
      //   {
      //     _hubManager->StartConnection();
      //   }
    }
  }
};

/**
 * @brief Constructor
 */
HubManager::HubManager(){};

void HubManager::Start()
{

  BLEDevice::init("");
  BLEScan *pBLEScan = BLEDevice::getScan();

  pBLEScan->setAdvertisedDeviceCallbacks(new HubManagerAdvertisedDeviceCallbacks(this));

  pBLEScan->setActiveScan(true);
  // start method with callback function to enforce the non blocking scan. If no callback function is used,
  // the scan starts in a blocking manner
  Serial.println("Start scan");
  pBLEScan->start(10);

  // if (nonBlocking)
  // {
  //   pBLEScan->start(scanDuration, HubManagerScanEndedCallback);
  // }
  // else
  // {
  //   pBLEScan->start(scanDuration);
  // }
}

void HubManager::Stop()
{
}

// void HubManager::StartScan(uint32_t scanDuration, bool nonBlocking) {

// }

// void HubManager::StartConnection()
// {
//   Serial.println("StartConnection");
//   for (int i = 0; i < ManagedHubs.size(); i++)
//   {
//     Serial.print("Connect Hub: ");
//     Serial.println(i, DEC);
//     ManagedHubs[i]->connectHub();
//   }
// }

void HubManager::AddHub(Lpf2Hub hub, std::string address, std::string name)
{
  hub._requestedDeviceAddress = new BLEAddress(address);
  hub._hubName = name;
  hub._isConnected = false;
  hub._isConnecting = false;
  hub._bleUuid = BLEUUID(LPF2_UUID);
  hub._charachteristicUuid = BLEUUID(LPF2_CHARACHTERISTIC);
  hub._hubType = HubType::UNKNOWNHUB;
  //hub.setHubName(name); // ToDo: DOES NOT WORK
  ManagedHubs.push_back(&hub);
}

Lpf2Hub* HubManager::GetHubByAddress(std::string address)
{
  return GetHubByAddress(NimBLEAddress(address));
}

Lpf2Hub* HubManager::GetHubByAddress(NimBLEAddress address)
{
  Serial.println("GetHubByAddress");
  for (int i = 0; i < ManagedHubs.size(); i++)
  {
    Serial.println(address.toString().c_str());
    NimBLEAddress hubAddress = *ManagedHubs[i]->_requestedDeviceAddress;
    if (address.equals(hubAddress))
    {
      Serial.println("found hub with requested address");
      return ManagedHubs[i];
    }
  }
  Serial.print("No hub found with address: ");
  Serial.println(address.toString().c_str());
  return nullptr;
}

Lpf2Hub* HubManager::GetHubByName(std::string name)
{
  Serial.println("GetHubByName");
  for (int i = 0; i < ManagedHubs.size(); i++)
  {
    Serial.print("Hub name: ");
    Serial.println(ManagedHubs[i]->getHubName().c_str());
    if (ManagedHubs[i]->getHubName() == name)
    {
      Serial.println("found hub with requested name");
      return ManagedHubs[i];
    }
  }
  Serial.print("No hub found with name: ");
  Serial.println(name.c_str());
  return nullptr;
}

//#endif // ESP32

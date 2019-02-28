#include "Lpf2Hub.h"

/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class AdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    Lpf2Hub* _lpf2Hub;
public:
    AdvertisedDeviceCallbacks(Lpf2Hub* lpf2Hub) : BLEAdvertisedDeviceCallbacks() {
        _lpf2Hub = lpf2Hub;
    }

    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        //Found a device, check if the service is contained
        if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(_lpf2Hub->_bleUuid))
        {
            advertisedDevice.getScan()->stop();
            _lpf2Hub->_pServerAddress = new BLEAddress(advertisedDevice.getAddress());
            _lpf2Hub->_isConnecting = true;
        }
    }
};
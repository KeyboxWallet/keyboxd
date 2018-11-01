#ifndef _KEYBOX_DEVICE_MANAGER_INCLUDE_
#define _KEYBOX_DEVICE_MANAGER_INCLUDE_
#include "base-device.hpp"

#define KEYBOX2_VENDOR_ID 0xb6ab
#define KEYBOX2_PRODUCT_ID 0xbaeb
#define KEYBOX2_BCD_DEVICE 0x0001

#include <boost/asio.hpp>
#include <set>

class DeviceEventListener {
public:
    virtual void deviceAdded(BaseDevice *) = 0;
    virtual void deviceRemoved(const std::string &dev_id) = 0;
};

class DeviceManager {

public:
    static DeviceManager * getDeviceManager(boost::asio::io_context *ioc);

    BaseDevice * getDeviceById(const std::string &id );
    std::vector<BaseDevice*> deviceList(); 
    void addDevice(BaseDevice *);
    void rmDevice(BaseDevice *);
    void registerEventListener(DeviceEventListener *listener);
    void unRegisterEventListener(DeviceEventListener *listener);

private:
    boost::asio::io_context * mContext;
    explicit DeviceManager(boost::asio::io_context *ioc);
    std::map<std::string, BaseDevice *> mDeviceMaps;
    std::set<DeviceEventListener *> mListeners;
    boost::asio::steady_timer *mTimer;
    void enumerateUsbDevice();
};

#endif

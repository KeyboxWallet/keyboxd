#include "device-manager.hpp"
#include "local-device.hpp"
#include "usb-device.hpp"
#include <time.h>
#include <set>

DeviceManager * globalManager = NULL;
DeviceManager *DeviceManager::getDeviceManager(boost::asio::io_context *ioc){
    if( !globalManager ) {
        globalManager = new DeviceManager(ioc);
    }
    return globalManager;
}

DeviceManager::DeviceManager(boost::asio::io_context *ioc){
    mContext = ioc;
    mTimer = new boost::asio::steady_timer(*ioc, boost::asio::chrono::seconds(1));
    mTimer->async_wait(std::bind( &DeviceManager::enumerateUsbDevice, this));
}

BaseDevice * DeviceManager::getDeviceById(const std::string &id )
{
    auto item = mDeviceMaps.find(id);
    if( item != mDeviceMaps.end()){
        return item->second;
    }
    return NULL;
}

std::vector<BaseDevice*> DeviceManager::deviceList()
{
    std::vector<BaseDevice*> r;
    for( auto elem: mDeviceMaps ){
        r.push_back(elem.second);
    }
    return r;
}

void DeviceManager::registerEventListener(DeviceEventListener *l)
{
    mListeners.insert(l);
}

void DeviceManager::unRegisterEventListener(DeviceEventListener *l)
{
    mListeners.erase(l);
}


void DeviceManager::addDevice(BaseDevice *d)
{
    mDeviceMaps[d->deviceId()] =  d;
    for( auto l : mListeners) {
        l->deviceAdded(d);
    }
}

void DeviceManager::rmDevice(BaseDevice *d)
{
    auto id = d->deviceId();
    mDeviceMaps.erase(id);
    for( auto l : mListeners) {
        l->deviceRemoved(id);
    }
}

void DeviceManager::enumerateUsbDevice()
{
    // 
    int dev_cnt = 0;
    libusb_device * keybox_devices[5];
    
    libusb_device **list;
    ssize_t cnt = libusb_get_device_list(NULL, &list);

    libusb_device *device;
    struct libusb_device_descriptor descriptor;
    
    //UsbDevice * usb_devices

    for(int i=0; i<cnt && dev_cnt < 5; i++){
        device = list[i];
        int err = libusb_get_device_descriptor(device, &descriptor);

        if (err) {
            continue;
        }
        if (    descriptor.idVendor == KEYBOX2_VENDOR_ID 
            &&  descriptor.idProduct == KEYBOX2_PRODUCT_ID 
            &&  descriptor.bcdDevice == KEYBOX2_BCD_DEVICE
            ) {
                keybox_devices[dev_cnt++] = device;
        }
    }

    // current usb devices
    std::set<std::string> keys;
    std::set<int> skip_indexes;
    UsbDevice *d;
    bool foundSame;
    for( auto elem: mDeviceMaps ){
        auto s = elem.second;
        if( s->deviceId().find("USB:") == 0) {
            d = dynamic_cast<UsbDevice*>(s);
            if (d->maybeDisconnected()){
                keys.insert(elem.first);
                continue;
            }
            foundSame = false;
            for(int i=0; i<dev_cnt && skip_indexes.find(i) == skip_indexes.end(); i++) {
                if( d->isSameDevice(keybox_devices[i])){
                    foundSame = true;
                    skip_indexes.insert(i);
                    break;
                }
            }
            if( !foundSame) {
                keys.insert(elem.first);
            }
        }
    }

    // delete devices disconnected
    for (auto key : keys) {
        auto dev = mDeviceMaps[key];
        rmDevice(dev);
        delete dev; 
    }

    // add device
    for (int i=0; i<dev_cnt && skip_indexes.find(i) == skip_indexes.end(); i++) {
        d = new UsbDevice(keybox_devices[i], mContext);
        if (!d->maybeDisconnected()){
            addDevice(d);
        }
        else {
            delete d;
        }
    }
    libusb_free_device_list(list, 1);
    mTimer->expires_at(mTimer->expiry() + boost::asio::chrono::seconds(5));
    mTimer->async_wait(std::bind( &DeviceManager::enumerateUsbDevice, this));
}
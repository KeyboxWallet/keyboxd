#include "base-device.hpp"

#define KEYBOX2_VENDOR_ID 0xb6ab
#define KEYBOX2_PRODUCT_ID 0xbaeb
#define KEYBOX2_BCD_DEVICE 0x0001

#include <boost/asio.hpp>
#include <libusb.h>

class DeviceManager {

public:
    static DeviceManager * getDeviceManager(boost::asio::io_context *ioc);

    BaseDevice * getDeviceById(const std::string &id );
    std::vector<BaseDevice*> deviceList(); 
    void addDevice(BaseDevice *);
    void rmDevice(BaseDevice *);

private:
    boost::asio::io_context * mContext;
    explicit DeviceManager(boost::asio::io_context *ioc);
    std::map<std::string, BaseDevice *> mDeviceMaps;
    boost::asio::steady_timer *mTimer;
    void enumerateUsbDevice();
};
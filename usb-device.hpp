#ifndef _KEYBOXD_USB_DEVICE_INCLUDE_
#define _KEYBOXD_USB_DEVICE_INCLUDE_

#include <boost/asio.hpp>

#include "base-device.hpp"
#include <libusb.h>

class UsbDevice: public BaseDevice {
public:
    explicit UsbDevice(libusb_device *device, boost::asio::io_context *ioc);

    virtual bool connect() ;
    virtual void disconnect() ;
    virtual bool isConnected() ;
    virtual const std::string deviceId();
    virtual void call_async(const std::string & method, const json & params, DevCallbackFn  cb) ; 
    ~UsbDevice();

    bool isSameDevice(libusb_device *device);
    bool maybeDisconnected();

    void writehandle( enum libusb_transfer_status ec, size_t length); // callback from usb only
    void readhandle( enum libusb_transfer_status ec, size_t length); // callback from usb only

private:
    libusb_device * mDevice;
    libusb_device_handle *mDevHandle;
    std::string mDeviceId;
    uint8_t mBus;
    uint8_t mPort;
    uint8_t mAddress;
    boost::asio::io_context *mContext;
    struct libusb_device_descriptor mDescriptor;
    uint8_t usb_write_pkg[1024];
    uint8_t usb_read_pkg[1024];
    int large_msg_type;
    int large_msg_len;
    uint8_t *large_msg;
    size_t large_msg_offset;
    struct libusb_transfer * mWriteTransfer;
    struct libusb_transfer * mReadTransfer;
    boost::asio::streambuf mBufferContent ;
    int32_t nextWriteOffset;
    bool mDisconnectedFlag;
    
    enum RpcStatus {
        IDLE,
        WRITING,
        READING_HEADER,
        READING_CONTENT,
    };
    RpcStatus mRpcStatus;
    std::string mCurrentMethod;
    DevCallbackFn mCb;

    void writeAckPackge();    
    void resetDevice();
    bool mResetFlag;
};

#endif

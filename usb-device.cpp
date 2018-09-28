#include "usb-device.hpp"
#include "keybox-proto-types.h"
#include "keybox-errcodes.h"
#include "messages.pb.h"
#include "base64.h"
#include "buffer-utils.hpp"
#include <sstream>
#include "device-manager.hpp"

#include <streambuf>

struct membuf : std::streambuf
{
    membuf(char *begin, char *end)
    {
        this->setg(begin, begin, end);
    }
};

UsbDevice::UsbDevice(libusb_device *device, boost::asio::io_context *ioc)
{
    //mServerPath = serverPath;
    mDevice = libusb_ref_device(device);
    // devId += "1";
    // struct libusb_device_descriptor descriptor;
    int err = libusb_get_device_descriptor(device, &mDescriptor);
    if (err)
    {
        //todo: fatal
        // libusb_unref_device(device);
        mDisconnectedFlag = true;
        return;
    }
    mBus = libusb_get_bus_number(device);
    mPort = libusb_get_port_number(device);
    mAddress = libusb_get_device_address(device);
    std::stringstream devId; //= "USB";
    devId << "USB:" << mBus << "_" << mPort << "_" << mAddress
          << "_" << mDescriptor.idVendor
          << "_" << mDescriptor.idProduct
          << "_" << mDescriptor.bcdDevice;

    mDeviceId = devId.str();
    mContext = ioc;
    mRpcStatus = IDLE;
    mDevHandle = NULL;
    mWriteTransfer = libusb_alloc_transfer(0);
    mReadTransfer = libusb_alloc_transfer(0);
    mDisconnectedFlag = false;
}

bool UsbDevice::maybeDisconnected()
{
    return mDisconnectedFlag;
}

UsbDevice::~UsbDevice()
{
    std::cerr << "~UsbDevice\n" ;
    if (mDevHandle)
    {
        libusb_close(mDevHandle);
        mDevHandle = NULL;
    }
    libusb_unref_device(mDevice);
    libusb_free_transfer(mWriteTransfer);
    libusb_free_transfer(mReadTransfer);
}

bool UsbDevice::isSameDevice(libusb_device *device)
{
    // todo: implem
    struct libusb_device_descriptor descriptor;
    int err = libusb_get_device_descriptor(device, &descriptor);

    if (err)
    {
        return false;
    }
    if (mBus != libusb_get_bus_number(device))
    {
        return false;
    }
    if (mPort != libusb_get_port_number(device))
    {
        return false;
    }
    if (mAddress != libusb_get_device_address(device))
    {
        return false;
    }

#define SAME_FIELD(fname)                      \
    if (mDescriptor.fname != descriptor.fname) \
        return false;

    SAME_FIELD(bcdUSB)
    SAME_FIELD(bDeviceClass)
    SAME_FIELD(bDeviceSubClass)
    SAME_FIELD(bDeviceProtocol)
    SAME_FIELD(idVendor)
    SAME_FIELD(idProduct)
    SAME_FIELD(bcdDevice)
    SAME_FIELD(bNumConfigurations)

    return true;
}

bool UsbDevice::connect()
{
    if (mDevHandle)
    {
        return true;
    }
    int r;
    r = libusb_open(mDevice, &mDevHandle);
    if (r)
    {
        std::cerr << "error open " << libusb_error_name(r) << "\n";
        return false;
    }

    r = libusb_kernel_driver_active(mDevHandle, 0);
    if (r && r != LIBUSB_ERROR_NOT_SUPPORTED)
    {
        r = libusb_detach_kernel_driver(mDevHandle, 0);
    }
    if (r && r != LIBUSB_ERROR_NOT_SUPPORTED)
    {
        std::cerr << "kernel active or detach " << libusb_error_name(r) << "\n";
        libusb_close(mDevHandle);
        mDevHandle = NULL;
        return false;
    }

    r = libusb_claim_interface(mDevHandle, 0);
    if (r)
    {
        std::cerr << "error claim " << libusb_error_name(r) << "\n";
        libusb_close(mDevHandle);
        mDevHandle = NULL;
        return false;
    }
    return true;
}

void UsbDevice::disconnect()
{
    if (!mDevHandle)
    {
        return;
    }
    if( mRpcStatus != IDLE) {
        libusb_cancel_transfer(mWriteTransfer);
        libusb_cancel_transfer(mReadTransfer);
        mRpcStatus = IDLE;
    }
    libusb_close(mDevHandle);
    mDevHandle = NULL;
}

bool UsbDevice::isConnected()
{
    return mDevHandle;
}

const std::string UsbDevice::deviceId()
{
    return mDeviceId;
}

extern "C"
{
    #ifndef NDEBUG
    void myToHex(uint8_t *srcbuf, int srcLen, char *dstBuff)
    {
        static char hexString[] = "0123456789ABCDEF";
        int i;
        for (i=0; i<srcLen; i++){
            dstBuff[2*i] = hexString[(srcbuf[i] & 0xF0) >> 4];
            dstBuff[2*i + 1] = hexString[srcbuf[i] & 0xF];
        }
        dstBuff[2*i] = 0;
    }
    #endif

    void LIBUSB_CALL writeComplete(struct libusb_transfer *transfer)
    {
        UsbDevice *dev = (UsbDevice *)transfer->user_data;
        boost::asio::post(std::bind(&UsbDevice::writehandle, dev, transfer->status, 1024));
    #ifndef NDEBUG
        char buff[32*2 + 1];
        myToHex(transfer->buffer, 32, buff);
        std::cout << "write handle "
            << transfer->status
            << " first 32 bytes: \n"
            << (char*)buff << "\n";
    #endif
    }

    void LIBUSB_CALL readComplete(struct libusb_transfer *transfer)
    {
        UsbDevice *dev = (UsbDevice *)transfer->user_data;
        boost::asio::post(std::bind(&UsbDevice::readhandle, dev, transfer->status, 1024));
   #ifndef NDEBUG
        char buff[32*2 + 1];
        myToHex(transfer->buffer, 32, buff);
        std::cout << "read handle "
            << transfer->status
            << " first 32 bytes: \n"
            << (char*)buff << "\n";
    #endif
    }
}

void UsbDevice::call_async(const std::string &method, const json &params, DevCallbackFn cb)
{
    json r;
    boost::system::error_code error;
    if (!mDevHandle)
    {
        return cb(KEYBOX_ERROR_CLIENT_ISSUE, "you should connect the device first", r);
    }
    if (mRpcStatus != IDLE)
    {
        return cb(KEYBOX_ERROR_CLIENT_ISSUE, "another call in progress.", r);
    }
    // write to socket

    std::ostream o_2(&mBufferContent);
    mCurrentMethod = method;

    uint32_t type;
    uint32_t len = 0;
    type = MsgTypeLowLimit;

    int32_t errCode;
    std::string errMessage;

    json_rpc_to_protobuf(method, params, errCode, errMessage, type, &o_2);

    if (errCode == 0)
    {
        type = htonl(type);
        len = htonl(mBufferContent.size());
        mRpcStatus = WRITING;
        //boost::asio::async_write(*mSocket, buffers,
        //                        std::bind(&UsbDevice::writehandle, this, cb, std::placeholders::_1, std::placeholders::_2));
        //

        bool multipkg = false;
        if (mBufferContent.size() < 1015)
        {                         // one packet message
            usb_write_pkg[0] = 1; //
        }
        else
        { // multi-package message
            usb_write_pkg[0] = 2;
            multipkg = true;
        }
        memcpy(usb_write_pkg + 1, (char *)&type, 4);
        memcpy(usb_write_pkg + 5, (char *)&len, 4);
        auto bufs = mBufferContent.data();

        int len = copy2RawBuffer(bufs, usb_write_pkg + 9, 1015);
        mBufferContent.consume(len);
        nextWriteOffset = len;
        mCb = cb;
#if TREZOR_TEST == 0
        libusb_fill_bulk_transfer(mWriteTransfer, mDevHandle, 2, usb_write_pkg, 1024, writeComplete, this, 2000);
#else
        libusb_fill_interrupt_transfer(mWriteTransfer, mDevHandle, 1, usb_write_pkg, 1024, writeComplete, this, 2000);
#endif
        errCode = libusb_submit_transfer(mWriteTransfer);

        if (errCode)
        {
            mRpcStatus = IDLE;
            return cb(errCode, "io error", r);
        }
    }
    else
    {
        cb(errCode, errMessage, r);
    }
}

void UsbDevice::writehandle(enum libusb_transfer_status ec, size_t length)
{
    json r;
    if (ec != LIBUSB_TRANSFER_COMPLETED)
    {
        mRpcStatus = IDLE;
		std::cerr << "write error " << libusb_error_name(ec) << "\n";
        mCb(KEYBOX_ERROR_SERVER_ISSUE, "io: write error", r);
        mDisconnectedFlag = true;
        /*
        if (ec == LIBUSB_TRANSFER_NO_DEVICE)
        {
            DeviceManager::getDeviceManager(NULL)->rmDevice(this);
        }*/
        return;
    }
    int readTimeout = 50000; // 50s
    if (mBufferContent.size())
    { // more pkg to send
        readTimeout = 2000;
    }

#if TREZOR_TEST == 0
    libusb_fill_bulk_transfer(mReadTransfer, mDevHandle, 129, usb_read_pkg, 1024, readComplete, this, readTimeout);
#else
    libusb_fill_interrupt_transfer(mReadTransfer, mDevHandle, 129, usb_read_pkg, 1024, readComplete, this, readTimeout);
#endif
    int errorCode;
    errorCode = libusb_submit_transfer(mReadTransfer);

    if (errorCode)
    {
        mRpcStatus = IDLE;
        return mCb(KEYBOX_ERROR_SERVER_ISSUE, "io error", r);
    }
}

void UsbDevice::writeAckPackge()
{
    json r;
    // write ack
    usb_write_pkg[0] = 4;
#if TREZOR_TEST == 0
    libusb_fill_bulk_transfer(mWriteTransfer, mDevHandle, 2, usb_write_pkg, 1024, writeComplete, this, 2000);
#else
    libusb_fill_interrupt_transfer(mWriteTransfer, mDevHandle, 1, usb_write_pkg, 1024, writeComplete, this, 2000);
#endif
    int errCode = libusb_submit_transfer(mWriteTransfer);

    if (errCode)
    {
        mRpcStatus = IDLE;
        return mCb(KEYBOX_ERROR_SERVER_ISSUE, "io error, submit writing", r);
    }
}

void UsbDevice::readhandle(enum libusb_transfer_status ec, size_t length)
{
    json r;
    if (mBufferContent.size())
    { // more pkg to send, write.

        if (usb_read_pkg[0] != 4)
        {
            mRpcStatus = IDLE;
            mCb(KEYBOX_ERROR_SERVER_ISSUE, "internal io error, multi-pkg write, reading ack", r);
            return;
        }
        //
        usb_write_pkg[0] = 3;
        int off = htonl(nextWriteOffset);
        memcpy(usb_write_pkg + 1, &off, 4);
        auto bufs = mBufferContent.data();

        int len = copy2RawBuffer(bufs, usb_write_pkg + 5, 1019);
        nextWriteOffset += len;
        mBufferContent.consume(len);
#if TREZOR_TEST == 0
        libusb_fill_bulk_transfer(mWriteTransfer, mDevHandle, 2, usb_write_pkg, 1024, writeComplete, this, 2000);
#else
        libusb_fill_interrupt_transfer(mWriteTransfer, mDevHandle, 1, usb_write_pkg, 1024, writeComplete, this, 2000);
#endif
        int errCode;
        errCode = libusb_submit_transfer(mWriteTransfer);

        if (errCode)
        {
            mRpcStatus = IDLE;
			std::cerr << "write submit transfer error " << libusb_error_name(r) << "\n";
            mCb(KEYBOX_ERROR_SERVER_ISSUE, "io error", r);
            return;
        }
    }
    else if (ec == LIBUSB_TRANSFER_COMPLETED)
    {
        // read  pkg ok
        if (mRpcStatus == WRITING)
        {
            mRpcStatus = READING_HEADER;

            /*parse first packet */
            if (length != 1024 || (usb_read_pkg[0] != 1 && usb_read_pkg[0] != 2))
            {
                mRpcStatus = IDLE;
                mCb(KEYBOX_ERROR_SERVER_ISSUE, "internal read issue", r);
                return;
            }
            int msgType, msgLen;
            memcpy(&msgType, usb_read_pkg + 1, 4);
            memcpy(&msgLen, usb_read_pkg + 5, 4);

            msgType = ntohl(msgType);
            msgLen = ntohl(msgLen);

            if( (usb_read_pkg[0] == 1 && msgLen > 1015)  ||
                (usb_read_pkg[0] == 2 && msgLen < 1015)
            ) {
                mRpcStatus = IDLE;
                mCb(KEYBOX_ERROR_SERVER_ISSUE, "protocol issue: pkg error from wallet", r);
                return;
            }

            if (msgLen <= 1015)
            {
                membuf m((char *)usb_read_pkg + 9, (char *)usb_read_pkg + 9 + msgLen);
                std::istream istream(&m);
                int32_t errCode;
                std::string errMessage;
                if (msgType == MsgTypeRequestRejected)
                {
                    RequestRejected rej;
                    if (rej.ParseFromIstream(&istream))
                    {
                        mRpcStatus = IDLE;
                        mCb(rej.errcode(), rej.errmessage(), r);
                        return;
                    }
                }
                protobuf_to_json_rpc(msgType, &istream, mCurrentMethod, errCode, errMessage, r);
                mRpcStatus = IDLE;
                mCb(errCode, errMessage, r);
            }
            else
            {
                large_msg_type = msgType;
                large_msg = new uint8_t[msgLen];
                memcpy(large_msg, usb_read_pkg + 9, 1015);
                large_msg_offset = 1015;
                mRpcStatus = READING_CONTENT;
                writeAckPackge();
            }
        }
        else if (mRpcStatus == READING_CONTENT)
        {
            // expected new
            if (length != 1024 || usb_read_pkg[0] != 3)
            {
                mRpcStatus = IDLE;
                mCb(KEYBOX_ERROR_SERVER_ISSUE, "internal read issue", r);
                return;
            }
            int msgOffset;
            memcpy(&msgOffset, usb_read_pkg + 1, 4);
            msgOffset = ntohl(msgOffset);

            if (msgOffset != large_msg_offset)
            {
                mRpcStatus = IDLE;
                mCb(KEYBOX_ERROR_SERVER_ISSUE, "internal read error: offset mismatch", r);
                return;
            }
            int remLen = large_msg_len - msgOffset;
            if (remLen <= 1019)
            { // final pkg
                memcpy(large_msg + msgOffset, usb_read_pkg + 5, remLen);
                membuf m((char *)large_msg, (char *)large_msg + large_msg_len);
                std::istream istream(&m);
                int32_t errCode;
                std::string errMessage;
                if (large_msg_type == MsgTypeRequestRejected)
                {
                    RequestRejected rej;
                    if (rej.ParseFromIstream(&istream))
                    {
                        mRpcStatus = IDLE;
                        mCb(rej.errcode(), rej.errmessage(), r);
                        return;
                    }
                    // return;
                }
                protobuf_to_json_rpc(large_msg_type, &istream, mCurrentMethod, errCode, errMessage, r);
                mRpcStatus = IDLE;
                mCb(errCode, errMessage, r);
            }
            else
            {
                memcpy(large_msg + msgOffset, usb_read_pkg + 5, 1019);
                large_msg_offset += 1019;
                writeAckPackge();
            }
        }
        else
        {
            mRpcStatus = IDLE;
            mCb(KEYBOX_ERROR_SERVER_ISSUE, "internal status issue", r);
            return;
        }
    }
    else if (ec == LIBUSB_TRANSFER_TIMED_OUT)
    { // read again
        std::cerr << "read timeout " << ec << "\n";
        int readTimeout = 50000;
#if TREZOR_TEST == 0
        libusb_fill_bulk_transfer(mReadTransfer, mDevHandle, 129, usb_read_pkg, 1024, readComplete, this, readTimeout);
#else
        libusb_fill_interrupt_transfer(mReadTransfer, mDevHandle, 129, usb_read_pkg, 1024, readComplete, this, readTimeout);
#endif
        int errCode;
        errCode = libusb_submit_transfer(mReadTransfer);

        if (errCode)
        {
            mRpcStatus = IDLE;
            return mCb(KEYBOX_ERROR_SERVER_ISSUE, "io error, submit to read", r);
        }
    }
    else
    {
        mRpcStatus = IDLE;
        mDisconnectedFlag = true;
        mCb(KEYBOX_ERROR_SERVER_ISSUE, "io error: read", r);
    }
}

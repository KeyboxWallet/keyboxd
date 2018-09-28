#ifndef _KEYBOXD_LOCAL_DEVICE_INCLUDE_
#define _KEYBOXD_LOCAL_DEVICE_INCLUDE_

#include <boost/asio.hpp>

#include "base-device.hpp"

class LocalDevice: public BaseDevice {
public:
    explicit LocalDevice(const std::string &serverPath, boost::asio::io_context *ioc);

    virtual bool connect() ;
    virtual void disconnect() ;
    virtual bool isConnected() ;
    virtual const std::string deviceId();
    virtual void call_async(const std::string & method, const json & params, DevCallbackFn  cb) ; 

private:
    std::string mServerPath;
    std::string mDeviceId;
    boost::asio::ip::tcp::socket *mSocket;
    boost::asio::io_context *mContext;
    boost::asio::streambuf mBufferHeader ;
    boost::asio::streambuf mBufferContent ;

    enum ConnectStatus{
        CLOSED,
        CONNECTING,
        CONNECTED
    };
    ConnectStatus mStatus;
    enum RpcStatus {
        IDLE,
        WRITING,
        READING_HEADER,
        READING_CONTENT,
    };
    RpcStatus mRpcStatus;
    std::string mCurrentMethod;
    DevCallbackFn mCb;
    void writehandle(DevCallbackFn cb, boost::system::error_code ec, size_t length);
    void readHeaderHandle(DevCallbackFn cb, boost::system::error_code ec, size_t length);
    void readContentHandle(DevCallbackFn cb, uint32_t messageType, boost::system::error_code ec, size_t length);

};

#endif

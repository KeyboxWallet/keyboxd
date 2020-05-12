#include "local-device.hpp"
#include "keybox-proto-types.h"
#include "keybox-errcodes.h"
#include "messages.pb.h"
#include "base64.h"
#include "buffer-utils.hpp"

LocalDevice::LocalDevice(const std::string &serverPath, boost::asio::io_context *ioc)
{
    mServerPath = serverPath;
    std::string devId = "local";
    devId += ":";
    devId += "1";
    mDeviceId = devId;
    mContext = ioc;
    mStatus = CLOSED;
    mRpcStatus = IDLE;
}

bool LocalDevice::connect()
{
    if (mStatus == CONNECTED)
    {
        return true;
    }
    mStatus = CONNECTING;

    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::make_address_v4("127.0.0.1"), 15320);
    mSocket = new boost::asio::ip::tcp::socket(*mContext);

    boost::system::error_code ec;
    mSocket->connect(ep, ec);
    if (ec)
    {
        mStatus = CLOSED;
        return false;
    }
    mStatus = CONNECTED;
    return true;
}

void LocalDevice::disconnect()
{
    if (mStatus == CLOSED)
    {
        return;
    }
    mSocket->close();
    mStatus = CLOSED;
}

bool LocalDevice::isConnected()
{
    return mStatus == CONNECTED;
}

const std::string LocalDevice::deviceId()
{
    return mDeviceId;
}

void LocalDevice::call_async(const std::string &method, const json &params, DevCallbackFn cb)
{
    json r;
    boost::system::error_code error;
    if (mStatus != CONNECTED)
    {
        return cb(KEYBOX_ERROR_CLIENT_ISSUE, "you should connect the device first", r);
    }
    if (mRpcStatus != IDLE)
    {
        return cb(KEYBOX_ERROR_CLIENT_ISSUE, "another call in progress.", r);
    }
    // write to socket

    std::ostream o_1(&mBufferHeader);
    std::ostream o_2(&mBufferContent);
    mCurrentMethod = method;

    uint32_t type;
    uint32_t len = 0;
    type = MsgTypeLowLimit;

    int32_t errCode;
    std::string errMessage;

    json_rpc_to_protobuf(method, params, errCode, errMessage, type, &o_2);

    if( errCode == 0 ) {
         type = htonl(type);
        o_1.write(reinterpret_cast<char *>(&type), 4);
        len = mBufferContent.data().size();
        std::cout << "len " << len << "\n";
        len = htonl(len);
        o_1.write(reinterpret_cast<char *>(&len), 4);
        std::vector<boost::asio::streambuf::const_buffers_type> buffers;
        buffers.push_back(mBufferHeader.data());
        buffers.push_back(mBufferContent.data());
        mRpcStatus = WRITING;
        boost::asio::async_write(*mSocket, buffers,
                                std::bind(&LocalDevice::writehandle, this, cb, std::placeholders::_1, std::placeholders::_2));
    }
    else {
        cb(errCode, errMessage, r);
    }
}

void LocalDevice::writehandle(DevCallbackFn cb, boost::system::error_code ec, size_t length)
{
    mBufferHeader.consume(mBufferHeader.size());
    mBufferContent.consume(mBufferContent.size());
    json r;
    if (ec)
    {
        mRpcStatus = IDLE;
        return cb(ec.value(), ec.message(), r);
    }
    else
    {
        mRpcStatus = READING_HEADER;
        auto buf = mBufferHeader.prepare(8);
        boost::asio::async_read(*mSocket, buf,
                                std::bind(&LocalDevice::readHeaderHandle, this, cb, std::placeholders::_1, std::placeholders::_2));
    }
}

void LocalDevice::readHeaderHandle(DevCallbackFn cb, boost::system::error_code ec, size_t length)
{
    mBufferHeader.commit(length);
    json r;
    if (ec)
    {
        mRpcStatus = IDLE;
        return cb(ec.value(), ec.message(), r);
    }
    uint32_t type;
    uint32_t len;

    uint8_t header[8];
    auto const_buffer = mBufferHeader.data();
    /*
    auto iter = boost::asio::buffer_sequence_begin(const_buffer);
    int l = 0;
    while (iter < boost::asio::buffer_sequence_end(const_buffer) && l < 8)
    {
        int size = iter->size();
        int copy_size = 8 - l;
        if (copy_size > size)
        {
            copy_size = size;
        }
        memcpy(header + l, iter->data(), copy_size);
        l += copy_size;
        iter++;
    }
    */
    int l = copy2RawBuffer(const_buffer, header, 8);
    memcpy(&type, header, 4);
    memcpy(&len, header + 4, 4);
    mBufferHeader.consume(length);

    type = htonl(type);
    len = htonl(len);

    if (mCurrentMethod == "getPublicKeyFromPath")
    {

        if (type != MsgTypeEccGetPublicKeyReply && type != MsgTypeRequestRejected)
        {
            mRpcStatus = IDLE;
            return cb(KEYBOX_ERROR_SERVER_ISSUE, "internal protocol error", r);
        }
    }
    else   if (mCurrentMethod == "getExtendedPubkeyFromPath")
    {

        if (type != MsgTypeEccGetExtendedPubkeyReply && type != MsgTypeRequestRejected)
        {
            mRpcStatus = IDLE;
            return cb(KEYBOX_ERROR_SERVER_ISSUE, "internal protocol error", r);
        }
    }
    else if( mCurrentMethod == "signReq") {
        if (type != MsgTypeEccSignResult && type != MsgTypeRequestRejected)
        {
            mRpcStatus = IDLE;
            return cb(KEYBOX_ERROR_SERVER_ISSUE, "internal protocol error", r);
        }     
    }
    else if( mCurrentMethod == "multiplyReq") {
        if (type != MsgTypeEccMultiplyReply && type != MsgTypeRequestRejected)
        {
            mRpcStatus = IDLE;
            return cb(KEYBOX_ERROR_SERVER_ISSUE, "internal protocol error", r);
        }     
    }

    mRpcStatus = READING_CONTENT;
    auto buf = mBufferContent.prepare(len);
    boost::asio::async_read(*mSocket, buf,
                            std::bind(&LocalDevice::readContentHandle, this, cb, type, std::placeholders::_1, std::placeholders::_2));
}

void LocalDevice::readContentHandle(DevCallbackFn cb, uint32_t messageType, boost::system::error_code ec, size_t length)
{
    mBufferContent.commit(length);
    std::istream istream(&mBufferContent);
    json r;
    mRpcStatus = IDLE;
    if (ec)
    {
        return cb(ec.value(), ec.message(), r);
    }

    if (messageType == MsgTypeRequestRejected)
    {
        RequestRejected rej;
        if (rej.ParseFromIstream(&istream))
        {
            return cb(rej.errcode(), rej.errmessage(), r);
        }
    }

    int32_t errCode;
    std::string errMessage;
    protobuf_to_json_rpc(messageType, &istream, mCurrentMethod, errCode, errMessage, r );
    // deserialze now
    cb(errCode, errMessage, r);
}

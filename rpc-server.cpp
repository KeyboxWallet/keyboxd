#include "rpc-server.hpp"
#include "keybox-errcodes.h"
struct rpc_session_state
{
    BaseDevice *bindDevice;
};

struct rpc_server_state
{
    std::map<generic_json_rpc_session *, rpc_session_state *> sessionStates;
    std::map<BaseDevice *, generic_json_rpc_session *> devStates;
};

rpc_server::rpc_server() : json_rpc_context_server()
{
    d = new rpc_server_state();
    DeviceManager *devManager = DeviceManager::getDeviceManager(NULL);
    devManager->registerEventListener(this);
}



void rpc_server::session_added(generic_json_rpc_session *session)
{
    rpc_session_state *state = new rpc_session_state();
    state->bindDevice = NULL;
    d->sessionStates[session] = state;
}

void rpc_server::session_removed(generic_json_rpc_session *session)
{
    rpc_session_state *state = d->sessionStates.at(session);
    d->sessionStates.erase(session);
    if (state->bindDevice)
    {
        auto bindDev = state->bindDevice;
        d->devStates.erase(state->bindDevice);
        state->bindDevice = NULL;
        bindDev->disconnect();
    }
}

void rpc_server::call(generic_json_rpc_session *session, const json &id, const std::string method_name, const json &params)
{
    json data;
    rpc_session_state *state = nullptr;
    try{
        state = d->sessionStates.at(session);
    }
    catch(std::exception e){

    }
    if (!state) {
        return genericReply(session, id, KEYBOX_ERROR_SERVER_ISSUE, "internal error: no state", data);
    }

    DeviceManager *devManager = DeviceManager::getDeviceManager(NULL);
    if (method_name == "getServerVersion") {
        data["version"] = KEYBOXD_SERVER_VERSION;
        return genericReply(session, id, 0, "", data);
    }
    if (method_name == "getDeviceList") {
        std::vector<BaseDevice *> devices = devManager->deviceList();
        data = json::array();
        for (auto dev : devices)
        {
            json j;
            j["deviceId"] = dev->deviceId();
            data.push_back(j);
        }
        return genericReply(session, id, 0, "", data);
    }
    if (method_name == "connectDevice") {
        std::string s = params;
        BaseDevice *dev = devManager->getDeviceById(s);
        // 检查本连接是否连接了一个设备
        if (state->bindDevice) {
            return genericReply(session, id, KEYBOX_ERROR_CLIENT_ISSUE, "should disconnect first", data);
        }
        if (dev)
        {
            // 检查一下该设备是否被占用
            if( d->devStates.find(dev) != d->devStates.end() ){
                return genericReply(session, id, KEYBOX_ERROR_DEVICE_BUSY, "device already used by another client", data);
            }

            bool result = dev->connect();
            if( result ) {
                d->sessionStates.at(session)->bindDevice = dev;
                d->devStates[dev] = session;
                return genericReply(session, id, 0, "connect ok", data);
            }
            else {
                return genericReply(session, id, KEYBOX_ERROR_SERVER_ISSUE, "connect failed", data);
            }
        }
        else
        {
            return genericReply(session, id, KEYBOX_ERROR_CLIENT_ISSUE, "no such device", data);
        }
    }
    if (method_name == "disconnectDevice") {
        std::string s = params;
        BaseDevice *dev = devManager->getDeviceById(s);
        // 检查本连接是否连接了一个设备
        if ( state->bindDevice != dev) {
            return genericReply(session, id, KEYBOX_ERROR_CLIENT_ISSUE, "should connect first", data);
        }
        dev->disconnect();
        d->devStates.erase(dev);
        state -> bindDevice = NULL;
        return genericReply(session, id, 0, "disconnect ok", data);
    }

    if (method_name == "getPublicKeyFromPath"
        || method_name == "getExtendedPubkeyFromPath"
        || method_name == "signReq"
        || method_name == "multiplyReq"
        || method_name == "getDeviceInfo"
        || method_name == "getWalletIdentifier"
        ) {
        if( !state->bindDevice ) {
            return genericReply(session, id, KEYBOX_ERROR_CLIENT_ISSUE, "you must connect dev first", data);
        }
        auto d_ = this->d;
        state->bindDevice->call_async(method_name, params, [session, id, d_](int32_t errCode, const std::string &errMessage, const json &result){
            rpc_session_state *state = nullptr;
            try{
                state = d_->sessionStates.at(session);
            }
            catch(std::exception e){

            }
            if ( state ) {
                json r;
                r["errcode"] = errCode;
                r["errmessage"] = errMessage;
                r["data"] = result;
                session->do_reply(id, r);
            }
        });
    }
    else{
        json r;
        r["errcode"] = KEYBOX_ERROR_CLIENT_ISSUE;
        r["errmessage"] = "unsupported message";
        r["data"] = json::object();
        session->do_reply(id, r);
    }
}

void rpc_server::genericReply(generic_json_rpc_session *session, const json &id, int32_t errCode, std::string errMessage, const json &data)
{
    json r;
    r["errcode"] = errCode;
    r["errmessage"] = errMessage;
    r["data"] = data;
    session->do_reply(id, r);
}

void rpc_server::deviceAdded(BaseDevice *device)
{
    json dev;
    dev["devId"] = device->deviceId();
    for(auto iter= d->sessionStates.begin(); iter != d->sessionStates.end(); iter++){
        auto session = iter->first;
        session->do_notify("device_added", dev);
    }
}
void rpc_server::deviceRemoved(const std::string &devId)
{
    json dev;
    dev["devId"] = devId;
    for(auto iter= d->sessionStates.begin(); iter != d->sessionStates.end(); iter++){
        auto session = iter->first;
        auto state = iter->second;
        session->do_notify("device_removed", dev);
        if( state->bindDevice != NULL && state->bindDevice->deviceId() == devId){
            d->devStates.erase(state->bindDevice);
            state->bindDevice = NULL;
        }
    }
}

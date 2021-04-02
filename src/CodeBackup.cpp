
void MsgToFrontEnd(mck::Message &msg)
{
    nlohmann::json j = msg;
    std::string out = "ReceiveMessage(\"" + j.dump() + "\");";
    std::cout << "Out: " << out << std::endl;
    if (m_guiReady.load()) {
        guiWindow.eval(out);
    }
}

template <typename T>
void MsgToGui(std::string section, std::string msgType, T &data)
{
    auto outMsg = mck::Message();
    outMsg.section = "data";
    outMsg.msgType = "full";
    nlohmann::json j = outMsg;
    j["data"] = data;
    std::string out = "ReceiveMessage(" + j.dump() + ");";
    if (m_guiReady.load()) {
        guiWindow.eval(out);
    }
}

std::string GetData(std::string msg)
{
    nlohmann::json jOut = m_config;
    return jOut.dump();
}

/*
std::string ChangePadValue(std::string msg)
{
    using json = nlohmann::json;
    std::vector<mck::PadData> messages;
    try {
        json j = json::parse(msg);
        messages = j.get<std::vector<mck::PadData>>();
    }
    catch (std::)
}*/

std::string SendMessage(std::string msg)
{
    m_guiReady = true;
    using json = nlohmann::json;
    std::vector<mck::Message> messages;
    bool update = false;
    try
    {
        json j = json::parse(msg);
        messages = j.get<std::vector<mck::Message>>();
    }
    catch (std::exception &e)
    {
        std::cerr << "Failed to convert incoming message: " << e.what() << std::endl;
        return e.what();
    }
    std::cout << msg << std::endl;

    for (unsigned i = 0; i < messages.size(); i++)
    {
        if (messages[i].section == "pads")
        {
            if (messages[i].msgType == "trigger")
            {
                std::unique_lock<std::mutex> lock(m_triggerMutex);
                while (m_triggerActive.load())
                {
                    m_triggerCond.wait(lock);
                }
                mck::TriggerData data = json::parse(messages[i].data);
                int triggerIdx = data.index;
                if (triggerIdx >= 0)
                {
                    std::cout << "Triggering PAD #" << (triggerIdx + 1) << std::endl;
                    m_trigger.push_back(std::pair<unsigned, double>(triggerIdx, data.strength));
                }
            }
            else if (messages[i].msgType == "change")
            {
                mck::PadData data = json::parse(messages[i].data);
                if (data.index >= 0 && data.index < m_config.numPads)
                {
                    if (data.type == "gain")
                    {
                        m_config.pads[data.index].gain = data.value;
                        update = true;
                    } else if (data.type == "sample")
                    {
                        unsigned idx = (unsigned)data.value;
                        if (idx < m_config.numSamples) {
                            m_config.pads[data.index].sampleIdx = (unsigned)data.value;
                            update = true;
                        }
                    }
                }
            }
        }
        else if (messages[i].section == "data")
        {
            if (messages[i].msgType == "get")
            {
                MsgToGui("data", "full", m_config);
            }
        }
    }

    if (update)
    {
        SP::VerifyConfiguration(m_config);
        MsgToGui("data", "full", m_config);
    }

    return "";
}

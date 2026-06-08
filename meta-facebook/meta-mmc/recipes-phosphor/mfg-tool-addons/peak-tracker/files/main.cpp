#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <variant>
#include <boost/asio.hpp>
#include <sdbusplus/asio/connection.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/bus/match.hpp>

std::map<std::string, double> peakTrackerMap;
std::unique_ptr<sdbusplus::asio::object_server> objectServer = nullptr;
std::map<std::string, std::shared_ptr<sdbusplus::asio::dbus_interface>> mfgInterfaces;

void handlePropertiesChanged(sdbusplus::message_t& msg) {
    std::string interfaceName;
    std::map<std::string, std::variant<double>> changedProperties;
    std::vector<std::string> invalidatedProperties;

    try {
        msg.read(interfaceName, changedProperties, invalidatedProperties);
    } catch (const std::exception& e) {
        return; 
    }

    if (interfaceName == "xyz.openbmc_project.Sensor.Value") {
        auto it = changedProperties.find("Value");
        if (it != changedProperties.end()) {
            double currentVoltage = std::get<double>(it->second);
            
            std::string sensorPath = msg.get_path(); 
            std::string sensorName = sensorPath.substr(sensorPath.find_last_of('/') + 1);

            if (currentVoltage > peakTrackerMap[sensorPath]) {
                peakTrackerMap[sensorPath] = currentVoltage;
                
                std::string myObjectPath = "/com/facebook/mfg/trackers/voltage_peak/" + sensorName;
                
                if (mfgInterfaces.find(myObjectPath) == mfgInterfaces.end() && objectServer) {
                    mfgInterfaces[myObjectPath] = objectServer->add_interface(myObjectPath, "com.facebook.mfg.Voltage");
                    mfgInterfaces[myObjectPath]->register_property("VoltagePeak", peakTrackerMap[sensorPath]);
                    mfgInterfaces[myObjectPath]->initialize();
                } else if (mfgInterfaces.find(myObjectPath) != mfgInterfaces.end()) {
                    mfgInterfaces[myObjectPath]->set_property("VoltagePeak", peakTrackerMap[sensorPath]);
                }
                
                std::cout << "[" << sensorName << "] New Peak! -> " << currentVoltage << " V\n";
            }
        }
    }
}

int main() {
    boost::asio::io_context io;
    auto systemBus = std::make_shared<sdbusplus::asio::connection>(io);

    systemBus->request_name("com.facebook.mfg.PeakTracker");
    objectServer = std::make_unique<sdbusplus::asio::object_server>(systemBus);

    auto matchSignal = std::make_unique<sdbusplus::bus::match_t>(
        *systemBus,
        "type='signal',"
        "sender='xyz.openbmc_project.ADCSensor'," 
        "interface='org.freedesktop.DBus.Properties',"
        "member='PropertiesChanged'",
        [](sdbusplus::message_t& msg) {
            handlePropertiesChanged(msg);
        }
    );

    std::cout << "Step 2: Monitoring multi-channel ADCSensor successfully...\n";

    io.run();
    return 0;
}
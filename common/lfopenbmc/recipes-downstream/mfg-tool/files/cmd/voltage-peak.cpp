#include "utils/json.hpp"
#include "utils/register.hpp"

#include <phosphor-logging/lg2.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <variant>
#include <sdbusplus/bus.hpp>

namespace mfgtool::cmds::voltage_peaks
{
PHOSPHOR_LOG2_USING;

struct command
{
    void init(CLI::App& app)
    {
        auto cmd = app.add_subcommand("voltage-peaks", "Get multi-channel ADC voltage peak values dynamically");
        
        init_callback(cmd, *this);
    }

    void run()
    {
        debug("Starting to fetch voltage peak values dynamically from PeakTracker...");

        auto bus = sdbusplus::bus::new_default();
        nlohmann::json result_json = nlohmann::json::object();

        try
        {
            auto mapper = bus.new_method_call(
                "xyz.openbmc_project.ObjectMapper",
                "/xyz/openbmc_project/object_mapper",
                "xyz.openbmc_project.ObjectMapper",
                "GetSubTreePaths"
            );
            
            mapper.append("/com/facebook/mfg/trackers/voltage_peak", 0, std::vector<std::string>{"com.facebook.mfg.Voltage"});
            auto mapperReply = bus.call(mapper);
            
            std::vector<std::string> foundPaths;
            mapperReply.read(foundPaths);

            if (foundPaths.empty())
            {
                json::display(js{{"status", "SUCCESS"}, {"peaks", "No peak data recorded yet"}});
                return;
            }

            for (const auto& objectPath : foundPaths)
            {
                std::string sensorName = objectPath.substr(objectPath.find_last_of('/') + 1);

                try
                {
                    auto method = bus.new_method_call(
                        "com.facebook.mfg.PeakTracker",
                        objectPath.c_str(),
                        "org.freedesktop.DBus.Properties",
                        "Get"
                    );
                    
                    method.append("com.facebook.mfg.Voltage", "VoltagePeak");
                    auto reply = bus.call(method);
                    
                    std::variant<double> peakValue;
                    reply.read(peakValue);
                    
                    result_json[sensorName] = std::get<double>(peakValue);
                }
                catch (const std::exception& e)
                {
                    result_json[sensorName] = "Error reading value";
                }
            }

            json::display(js{{"status", "SUCCESS"}, {"peaks", result_json}});
        }
        catch (const std::exception& e)
        {
            json::display(js{{"status", "ERROR"}, {"reason", "PeakTracker service not active or mapper failed"}});
        }
    }
};

MFGTOOL_REGISTER(command);

}
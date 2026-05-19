#include "utils/json.hpp"
#include "utils/register.hpp"

#include <phosphor-logging/lg2.hpp>
#include <iostream>

namespace mfgtool::cmds::hello_mmc
{
PHOSPHOR_LOG2_USING;

struct command
{
    void init(CLI::App& app)
    {
        auto cmd = app.add_subcommand("hello_mmc", "MMC test hello command");
        
        init_callback(cmd, *this);
    }
    void run()
    {
        debug("Starting MMC test hello logic...");

        json::display(js{"MMC test hello"});
    }
};

MFGTOOL_REGISTER(command);

} // namespace mfgtool::cmds::hello_mmc
#include <ignition/plugin/Register.hh>
#include "GuiPlotting.hh"

using namespace ignition;
using namespace gui;
using namespace plugins;

GuiPlotting::~GuiPlotting()
{
}
//////////////////////////////////////////
GuiPlotting::GuiPlotting()  : Plugin()
{
//    this->plot = new ignition::gui::PlottingInterface();
}

// Register this plugin
IGNITION_ADD_PLUGIN(ignition::gui::plugins::GuiPlotting,
                    ignition::gui::Plugin)


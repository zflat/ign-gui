#ifndef IGNITION_GUI_PLUGINS_GuiPlotting_HH_
#define IGNITION_GUI_PLUGINS_GuiPlotting_HH_

#include <ignition/gui/Application.hh>
#include <ignition/gui/Plugin.hh>
#include <ignition/gui/qt.h>

#include <ignition/gui/PlottingInterface.hh>

//ignition::gui::PlottingInterface xxxxxxxx;

namespace ignition
{
namespace gui
{
namespace plugins
{

class GuiPlotting : public ignition::gui::Plugin
{
    Q_OBJECT

    public: GuiPlotting();
    public: ~GuiPlotting();

    public: ignition::gui::PlottingInterface plot;
};

}
}
}
#endif

#include <JuceHeader.h>
#include "MainComponent.hpp"

class Application : public juce::JUCEApplication
{
public:
    Application() = default;

    const juce::String getApplicationName() override { return "MckSampler"; }
    const juce::String getApplicationVersion() override { return "0.0.1"; }

    void initialise(const juce::String &) override
    {
        mainWindow.reset(new MainWindow(getApplicationName(), new MainComponent(), *this));
    }

    void shutdown() override {}

private:
    class MainWindow : public juce::DocumentWindow
    {
    public:
        MainWindow(const juce::String &name, juce::Component *c, JUCEApplication &a)
            : DocumentWindow(name,
                             juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(DocumentWindow::backgroundColourId),
                             juce::DocumentWindow::minimiseButton | juce::DocumentWindow::closeButton),
              app(a)
        {
            setUsingNativeTitleBar(false);
            setResizable(false, false);
            setSize(800, 480);
            setContentOwned(c, true);
            centreWithSize(getWidth(), getHeight());
            setVisible(true);
        }

        void closeButtonPressed() override
        {
            app.systemRequestedQuit();
        }

    private:
        JUCEApplication &app;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(Application)
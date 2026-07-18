#pragma once
#include <juce_audio_utils/juce_audio_utils.h>

/*
    Minimal preset system: each preset is an .xml file in
    Documents/Boogie Labs/Atlas Presets. The < and > buttons in the
    editor cycle through them; Save writes the current knob state.
*/
class PresetManager
{
public:
    explicit PresetManager (juce::AudioProcessorValueTreeState& state)
        : apvts (state)
    {
        directory.createDirectory();   // make the folder if it doesn't exist
    }

    void save (const juce::String& name)
    {
        if (name.isEmpty()) return;

        if (auto xml = apvts.copyState().createXml())
            xml->writeTo (directory.getChildFile (name + ".xml"));

        currentName = name;
        refreshList();
    }

    void loadNext()     { step ( 1); }
    void loadPrevious() { step (-1); }

    juce::String getCurrentName() const { return currentName; }

private:
    void refreshList()
    {
        files = directory.findChildFiles (juce::File::findFiles, false, "*.xml");
        files.sort();
    }

    void step (int direction)
    {
        refreshList();
        if (files.isEmpty()) return;

        // find where we are, move one step, wrap around the ends
        int index = 0;
        for (int i = 0; i < files.size(); ++i)
            if (files[i].getFileNameWithoutExtension() == currentName)
                index = i;

        index = (index + direction + files.size()) % files.size();
        load (files[index]);
    }

    void load (const juce::File& file)
    {
        if (auto xml = juce::XmlDocument::parse (file))
        {
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
            currentName = file.getFileNameWithoutExtension();
        }
    }

    juce::AudioProcessorValueTreeState& apvts;
    juce::File directory = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                               .getChildFile ("Boogie Labs").getChildFile ("Atlas Presets");
    juce::Array<juce::File> files;
    juce::String currentName = "Init";
};

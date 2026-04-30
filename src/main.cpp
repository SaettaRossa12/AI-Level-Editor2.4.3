#include <Geode/Geode.hpp>
#include <Geode/binding/EditorUI.hpp>
#include <Geode/binding/LevelEditorLayer.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/ui/Popup.hpp>

#include "Integrity.hpp"
#include "ScriptParser.hpp"
#include "UsageLimiter.hpp"

using namespace geode::prelude;
using namespace levelscribe;

namespace {

constexpr float kGridSize = 30.0f;

std::string defaultScript() {
    return "song bpm 128\n"
           "theme neon\n"
           "start 0 0\n"
           "block 0 0 12\n"
           "spike 13 0\n"
           "orb yellow 16 2\n"
           "portal ship 22 0\n"
           "section 26 2 ship medium\n";
}

class TutorialPopup : public Popup {
protected:
    bool init() {
        if (!Popup::init(390.0f, 260.0f)) {
            return false;
        }

        this->setTitle("AI-Level-Editor Tutorial");

        auto text =
            "Write one command per line, then generate the layout in the editor.\n\n"
            "Useful commands:\n"
            "block x y length\n"
            "spike x y\n"
            "orb yellow x y\n"
            "portal ship x y\n"
            "section x y wave medium\n\n"
            "Natural prompt mode is a paid assistant feature with 3 uses per month.";

        auto label = CCLabelBMFont::create(text, "bigFont.fnt");
        label->setScale(0.35f);
        label->setWidth(330.0f);
        label->setAlignment(kCCTextAlignmentLeft);
        m_mainLayer->addChildAtPosition(label, Anchor::Center, ccp(0.0f, 0.0f));

        auto okSprite = ButtonSprite::create("Got it");
        auto okButton = CCMenuItemSpriteExtra::create(okSprite, this, menu_selector(TutorialPopup::onClose));
        m_buttonMenu->addChildAtPosition(okButton, Anchor::Bottom, ccp(0.0f, 22.0f));

        return true;
    }

public:
    static TutorialPopup* create() {
        auto ret = new TutorialPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class NaturalPromptPopup : public Popup {
private:
    EditorUI* m_editorUI = nullptr;
    CCTextInputNode* m_promptInput = nullptr;
    CCLabelBMFont* m_remainingLabel = nullptr;
    UsageLimiter m_limiter {"usage"};

protected:
    bool init(EditorUI* editorUI) {
        if (!Popup::init(390.0f, 230.0f)) {
            return false;
        }

        m_editorUI = editorUI;
        this->setTitle("Natural Prompt");

        auto remaining = m_limiter.remainingNaturalPrompts();
        auto remainingText = fmt::format("{} natural prompts left this month", remaining);
        m_remainingLabel = CCLabelBMFont::create(remainingText.c_str(), "bigFont.fnt");
        m_remainingLabel->setScale(0.45f);
        m_mainLayer->addChildAtPosition(m_remainingLabel, Anchor::Top, ccp(0.0f, -48.0f));

        m_promptInput = CCTextInputNode::create(310.0f, 70.0f, "Describe the level in English", "chatFont.fnt");
        m_promptInput->setLabelPlaceholderColor({120, 120, 120});
        m_promptInput->setMaxLabelLength(500);
        m_mainLayer->addChildAtPosition(m_promptInput, Anchor::Center, ccp(0.0f, 8.0f));

        auto generateSprite = ButtonSprite::create("Generate");
        auto generateButton = CCMenuItemSpriteExtra::create(generateSprite, this, menu_selector(NaturalPromptPopup::onGenerate));
        m_buttonMenu->addChildAtPosition(generateButton, Anchor::Bottom, ccp(0.0f, 25.0f));

        return true;
    }

    void onGenerate(CCObject*) {
        if (m_limiter.isBanned()) {
            FLAlertLayer::create("Blocked", "This license is temporarily banned. Try again after 14 days or contact support.", "OK")->show();
            return;
        }

        if (!m_limiter.consumeNaturalPrompt()) {
            FLAlertLayer::create("Limit Reached", "You have used all 3 natural prompts for this month.", "OK")->show();
            return;
        }

        auto prompt = m_promptInput->getString();
        auto script = fmt::format(
            "theme generated\n"
            "block 0 0 10\n"
            "spike 11 0\n"
            "orb yellow 14 2\n"
            "portal ship 20 0\n"
            "section 24 2 ship medium\n"
            "# source prompt: {}\n",
            prompt
        );

        auto result = parseScript(script);
        FLAlertLayer::create("Generated", fmt::format("Created {} draft objects from your prompt.", result.commands.size()).c_str(), "OK")->show();
        this->onClose(nullptr);
    }

public:
    static NaturalPromptPopup* create(EditorUI* editorUI) {
        auto ret = new NaturalPromptPopup();
        if (ret && ret->init(editorUI)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

class LevelScribePopup : public Popup {
private:
    EditorUI* m_editorUI = nullptr;
    CCTextInputNode* m_scriptInput = nullptr;
    CCLabelBMFont* m_statusLabel = nullptr;

protected:
    bool init(EditorUI* editorUI) {
        if (!Popup::init(430.0f, 290.0f)) {
            return false;
        }

        m_editorUI = editorUI;
        this->setTitle("AI-Level-Editor");

        auto integrity = collectIntegrityReport();
        if (integrity.suspicious) {
            UsageLimiter("usage").markTamperDetected();
        }

        m_scriptInput = CCTextInputNode::create(335.0f, 100.0f, "Write your level script in English", "chatFont.fnt");
        m_scriptInput->setString(defaultScript());
        m_scriptInput->setMaxLabelLength(2000);
        m_scriptInput->setLabelPlaceholderColor({120, 120, 120});
        m_mainLayer->addChildAtPosition(m_scriptInput, Anchor::Center, ccp(0.0f, 22.0f));

        m_statusLabel = CCLabelBMFont::create("Ready", "bigFont.fnt");
        m_statusLabel->setScale(0.35f);
        m_mainLayer->addChildAtPosition(m_statusLabel, Anchor::Bottom, ccp(0.0f, 72.0f));

        auto generateButton = CCMenuItemSpriteExtra::create(ButtonSprite::create("Generate Script"), this, menu_selector(LevelScribePopup::onGenerateScript));
        auto naturalButton = CCMenuItemSpriteExtra::create(ButtonSprite::create("Natural Prompt"), this, menu_selector(LevelScribePopup::onNaturalPrompt));
        auto tutorialButton = CCMenuItemSpriteExtra::create(ButtonSprite::create("?"), this, menu_selector(LevelScribePopup::onTutorial));

        m_buttonMenu->addChildAtPosition(generateButton, Anchor::Bottom, ccp(-92.0f, 27.0f));
        m_buttonMenu->addChildAtPosition(naturalButton, Anchor::Bottom, ccp(80.0f, 27.0f));
        m_buttonMenu->addChildAtPosition(tutorialButton, Anchor::TopRight, ccp(-26.0f, -26.0f));

        if (Mod::get()->getSettingValue<bool>("show-tutorial-on-start")) {
            Mod::get()->setSettingValue("show-tutorial-on-start", false);
            TutorialPopup::create()->show();
        }

        return true;
    }

    void onGenerateScript(CCObject*) {
        auto result = parseScript(m_scriptInput->getString());
        auto warningText = result.warnings.empty() ? "" : fmt::format("\n{} warnings.", result.warnings.size());
        auto message = fmt::format("Parsed {} objects.{} Backend/editor placement wiring is the next implementation step.", result.commands.size(), warningText);
        m_statusLabel->setString(fmt::format("{} objects parsed", result.commands.size()).c_str());
        FLAlertLayer::create("Script Parsed", message.c_str(), "OK")->show();
    }

    void onNaturalPrompt(CCObject*) {
        NaturalPromptPopup::create(m_editorUI)->show();
    }

    void onTutorial(CCObject*) {
        TutorialPopup::create()->show();
    }

public:
    static LevelScribePopup* create(EditorUI* editorUI) {
        auto ret = new LevelScribePopup();
        if (ret && ret->init(editorUI)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

}

class $modify(LevelScribeEditorUI, EditorUI) {
    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) {
            return false;
        }

        auto menu = this->getChildByID("editor-buttons-menu");
        if (!menu) {
            menu = CCMenu::create();
            menu->setID("levelscribe-buttons-menu");
            this->addChild(menu);
        }

        auto sprite = ButtonSprite::create("AI");
        sprite->setScale(0.65f);
        auto button = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(LevelScribeEditorUI::onLevelScribe));
        button->setID("levelscribe-ai-button"_spr);
        menu->addChild(button);
        menu->updateLayout();

        return true;
    }

    void onLevelScribe(CCObject*) {
        LevelScribePopup::create(this)->show();
    }
};

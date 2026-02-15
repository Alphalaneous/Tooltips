#include <Geode/Geode.hpp>
#include <alphalaneous.alphas_geode_utils/include/ObjectModify.hpp>
#include <alphalaneous.alphas_geode_utils/include/Utils.hpp>

using namespace geode::prelude;

static std::unordered_map<std::string, std::string> g_replacements = {
	{"dankmeme.globed2/main-menu-button", "Globed"},
	{"cvolton.betterinfo/main-button", "BetterInfo"},
	{"techstudent10.gdguesser/start-btn", "GDGuesser"},
};

static std::unordered_map<std::string, std::string> g_simpleReplacements = {
	{"id", "ID"},
	{"gdutils", "GDUtils"},
	{"gd", "GD"},
	{"youtube", "YouTube"},
	{"robtop", "RobTop"},
	{"hsv", "HSV"},
	{"ui", "UI"},
};

class TooltipNode : public CCNodeRGBA {
protected:
    CCLabelBMFont* m_label = nullptr;
    geode::NineSlice* m_bg = nullptr;
    CCNode* m_currentActiveNode = nullptr;
    float m_nodeScale = 1.f;
    float m_offset;
	ccColor3B m_nodeColor;
public:
    static TooltipNode* create() {
        auto ret = new TooltipNode();
        if (ret->init()) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    bool init() {
        m_bg = geode::NineSlice::create("square02b_001.png");
        m_bg->setScale(0.5f);
        m_bg->setColor({0, 0, 0});
        m_bg->setOpacity(200);

        m_label = CCLabelBMFont::create("", "chatFont.fnt");

        addChild(m_bg);
        addChild(m_label);

        setAnchorPoint({0.5f, 0.5f});
        setScale(0.4f);
        m_bg->setOpacity(0);
        m_label->setOpacity(0);
        return true;
    }

    static void uppercaseFirstChar(std::string& str) {
        if (!str.empty()) {
            str[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(str[0])));
        }
    }

    std::string deduceExpectedName(CCNode* node, const std::string& text) {
		for (const auto& [k, v] : g_replacements) {
			if (k == text) return v;
		}

        const auto& splitSlash = utils::string::split(text, "/");
        if (splitSlash.empty()) return "";

        const auto& finalPart = splitSlash.back();
        auto splitDash = utils::string::split(finalPart, "-");
		if (splitDash.size() == 1) {
        	splitDash = utils::string::split(finalPart, "_");
		}

        std::string ret;
        for (auto& str : splitDash) {
            if (str == "button" || str == "btn") continue;

			for (const auto& [k, v] : g_simpleReplacements) {
				if (k == str) str = v;
			}

            uppercaseFirstChar(str);
            ret += str + " ";
        }
        if (!ret.empty()) ret.pop_back();
        return ret;
    }

    void setLabel(const std::string& text) {
        if (text.empty()) return;

        m_label->setString(text.c_str());

        constexpr int paddingW = 8;
        constexpr int paddingH = 6;
        auto labelSize = m_label->getContentSize();

        m_bg->setContentSize((labelSize + CCSize{paddingW, paddingH}) * 2);
        setContentSize(labelSize + CCSize{paddingW, paddingH});

        auto center = getContentSize() / 2;
        m_label->setPosition(center);
        m_bg->setPosition(center);
    }

    void hideTooltip() {
        removeFromParentAndCleanup(false);
    }

    static CCNode* getSceneChildContainingNode(CCNode* node) {
        if (!node) return nullptr;
        auto current = node;
        while (current && current->getParent() != CCScene::get()) {
            current = current->getParent();
        }
        return current;
    }

    bool isLastAlert(CCNode* node) {
        bool shouldCheck = false;
        bool lastAlert = false;

        if (auto child = getSceneChildContainingNode(node)) {
            for (auto c : CCArrayExt<CCNode*>(child->getChildren())) {
                if (!alpha::utils::cocos::hasNode(node, c)) {
                    shouldCheck = true;
                }
                if (shouldCheck) {
                    if (typeinfo_cast<FLAlertLayer*>(c) || typeinfo_cast<CCBlockLayer*>(c)) {
                        if (alpha::utils::cocos::hasNode(node, c)) continue;
                        lastAlert = true;
                    }
                }
            }
        }
        return lastAlert;
    }

    bool isHoverable(CCNode* node, CCPoint point) {
        if (!CCScene::get() || !node || isLastAlert(node)) return false;

        auto sceneChild = getSceneChildContainingNode(node);
        if (!sceneChild) return false;

        if (node->getChildrenCount() == 1) {
            if (typeinfo_cast<ButtonSprite*>(node->getChildren()->objectAtIndex(0))) {
                return false;
            }
        }

        for (auto child : CCArrayExt<CCNode*>(CCScene::get()->getChildren())) {
            if (child->getZOrder() <= sceneChild->getZOrder()) continue;
            if (child->boundingBox().containsPoint(point) && nodeIsVisible(child)) {
                return false;
            }
        }
        return true;
    }

    void show(float) {
        m_bg->runAction(CCFadeTo::create(0.1f, 200));
        m_label->runAction(CCFadeTo::create(0.1f, 255));

        auto currentPos = getPosition();

        runAction(CCMoveTo::create(0.1f, {currentPos.x, currentPos.y + m_offset}));
        runAction(CCScaleTo::create(0.1f, 0.5f));
    }

    void showTooltip(CCNode* node, const std::string& text) {
		auto item = typeinfo_cast<CCMenuItem*>(node);
        if (!item) return;
#ifdef GEODE_IS_DESKTOP
		if (item->isSelected()) {
			return removeFromParentAndCleanup(false);
		}
#endif
        if (m_currentActiveNode == node && getParent()) return;

        removeFromParentAndCleanup(false);
        auto str = typeinfo_cast<CCString*>(node->getUserObject("tooltip"_spr));
        if (!node || (text.empty() && (!str || std::string_view(str->getCString()).empty()))) return;

        setScale(0.4f);
        m_bg->setOpacity(0);
        m_label->setOpacity(0);
        m_currentActiveNode = node;
        auto worldPos = node->convertToWorldSpaceAR(CCPointZero);
        auto labelText = str ? str->getCString() : deduceExpectedName(node, text);
        if (labelText.empty()) return;

        if (!isHoverable(node, worldPos)) return;

        CCScene::get()->addChild(this);
        setZOrder(CCScene::get()->getHighestChildZ() + 1);
        setLabel(labelText);

        auto winSize = CCDirector::get()->getWinSize();
        auto nodeWorldSize = node->boundingBox().size;

        float heightOffset = nodeWorldSize.height / 2;
        bool negativeOffset = false;

        constexpr float realScale = 0.5f;
        auto realSize = getContentSize() * realScale;

#ifdef GEODE_IS_DESKTOP
        m_offset = 6.f;
#else
        m_offset = 15.f;
#endif
        if ((heightOffset + realSize.height / 2 + worldPos.y + m_offset) > winSize.height) {
            heightOffset *= -1;
            negativeOffset = true;
#ifdef GEODE_IS_DESKTOP
            m_offset = -6.f;
#else
            m_offset = -15.f;
#endif
        }

        worldPos.y += heightOffset;

        constexpr int sidePadding = 2;
        if (worldPos.x - realSize.width / 2 < sidePadding) 
            worldPos.x = realSize.width / 2 + sidePadding;
        if (worldPos.x + realSize.width / 2 > winSize.width - sidePadding) 
            worldPos.x = winSize.width - sidePadding - realSize.width / 2;

        m_bg->stopAllActions();
        m_label->stopAllActions();
        stopAllActions();

        setPosition(worldPos);
		
#ifdef GEODE_IS_DESKTOP
        scheduleOnce(schedule_selector(TooltipNode::show), 0.4f);
#else
        scheduleOnce(schedule_selector(TooltipNode::show), 0.1f);
#endif
    }
};

class $nodeModify(MyCCMenu, cocos2d::CCMenu) {
    struct Fields {
        Ref<TooltipNode> m_tooltipNode;
        ~Fields() { m_tooltipNode->hideTooltip(); }
    };

    void modify() {
        auto fields = m_fields.self();
        fields->m_tooltipNode = TooltipNode::create();
        schedule(schedule_selector(MyCCMenu::checkMouse));
    }

    void checkMouse(float) {
        if (!nodeIsVisible(this)) return;
        auto fields = m_fields.self();
        bool shown = false;

#ifdef GEODE_IS_DESKTOP
        auto mousePos = getMousePos();
        auto local = convertToNodeSpace(mousePos);
        for (auto child : CCArrayExt<CCNode*>(getChildren())) {
            if (!nodeIsVisible(child)) continue;
            if (child->boundingBox().containsPoint(local)) {
                fields->m_tooltipNode->showTooltip(child, child->getID());
                shown = true;
            }
        }
#else
        auto self = reinterpret_cast<CCMenu*>(this);
        if (auto item = self->m_pSelectedItem) {
            if (item->isSelected()) {
                fields->m_tooltipNode->showTooltip(item, item->getID());
                shown = true;
            }
        }
#endif
        if (!shown) fields->m_tooltipNode->hideTooltip();
    }
};

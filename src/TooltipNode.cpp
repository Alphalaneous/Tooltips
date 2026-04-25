#include "TooltipNode.hpp"
#include <Geode/ui/Button.hpp>
#include "Utils.hpp"

using namespace tooltips;

TooltipNode* TooltipNode::s_instance = nullptr;

TooltipNode* TooltipNode::create() {
    auto ret = new TooltipNode();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

TooltipNode* TooltipNode::get() {
    if (!s_instance) s_instance = TooltipNode::create();
    return s_instance;
}

void TooltipNode::reset() {
    std::set<CCNode*> nodes;
    if (s_instance) {
        nodes = s_instance->getActiveNodes();
        s_instance->removeFromParent();
        s_instance = nullptr;
    }
    OverlayManager::get()->addChild(tooltips::TooltipNode::get());
    tooltips::TooltipNode::get()->setActiveNodes(nodes);
}

std::set<CCNode*> TooltipNode::getActiveNodes() {
    return m_activeNodes;
}

void TooltipNode::setActiveNodes(std::set<CCNode*> nodes) {
    m_activeNodes = std::move(nodes);
}

void TooltipNode::addNode(CCNode* node) {
    if (!node) return;
    m_activeNodes.insert(node);
}

void TooltipNode::removeNode(CCNode* node) {
    if (!node) return;
    if (m_currentNode == node) {
        setVisible(false);
    }
    m_activeNodes.erase(node);
}

void TooltipNode::setLabel(ZStringView text) {
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

void TooltipNode::show(float dt) {
    m_bg->runAction(CCFadeTo::create(0.1f, 200));
    m_label->runAction(CCFadeTo::create(0.1f, 255));

    auto currentPos = getPosition();

    runAction(CCMoveTo::create(0.1f, {currentPos.x, currentPos.y + m_offset}));
    runAction(CCScaleTo::create(0.1f, 0.5f));
}

void TooltipNode::showTooltip(CCNode* node) {
    if (m_currentNode == node) return;
    m_currentNode = node;

    auto str = typeinfo_cast<CCString*>(node->getUserObject("tooltip"_spr));

    std::string text;
    if (str) text = str->getCString();
    else text = utils::deduceExpectedName(node);

    if (text.empty()) return;

    setAnchorPoint({0.5f, 0.f});
    setScale(0.4f);
    m_bg->setOpacity(0);
    m_label->setOpacity(0);

    auto worldPos = node->convertToWorldSpaceAR(CCPointZero);

    setVisible(true);
    setLabel(text);

    auto winSize = CCDirector::get()->getWinSize();
    auto boundSize = node->boundingBox().size;

    m_bg->stopAllActions();
    m_label->stopAllActions();
    stopAllActions();

#ifdef GEODE_IS_DESKTOP
    m_offset = 3.f;
#else
    m_offset = 10.f;
#endif

    float edgePadding = 10.f;

    auto pos = worldPos + CCPoint{0, boundSize.height / 2};

    pos.x = std::min(std::max(edgePadding + getScaledContentWidth() / 2, pos.x), winSize.width - edgePadding - getScaledContentWidth() / 2);
    
    if (pos.y > winSize.height - edgePadding - getScaledContentHeight() / 2) {
        pos.y = worldPos.y - boundSize.height / 2;
        m_offset *= -1;
        setAnchorPoint({0.5f, 1.f});
    }

    setPosition(pos);
    
#ifdef GEODE_IS_DESKTOP
    scheduleOnce(schedule_selector(TooltipNode::show), 0.4f);
#else
    scheduleOnce(schedule_selector(TooltipNode::show), 0.1f);
#endif
}

void TooltipNode::checkMouse(float dt) {
    auto mousePos = getMousePos();

    bool tooltipShown = false;

    for (auto node : m_activeNodes) {
        auto parent = node->getParent();
        if (!parent) continue;

        #ifdef GEODE_IS_DESKTOP

        auto local = parent->convertToNodeSpace(mousePos);
        if (node->boundingBox().containsPoint(local) && tooltips::utils::isHoverable(node, mousePos)) {
            showTooltip(node);
            tooltipShown = true;
            break;
        }

        #else

        auto item = typeinfo_cast<CCMenuItem*>(node);
        if (item && item->isSelected()) {
            showTooltip(node);
            tooltipShown = true;
            break;
        }

        auto button = typeinfo_cast<geode::Button*>(node);
        if (button && button->isSelected()) {
            showTooltip(node);
            tooltipShown = true;
            break;
        }

        #endif
    }

    if (!tooltipShown) {
        m_currentNode = nullptr;
        setVisible(false);
    }
}

bool TooltipNode::init() {
    m_bg = geode::NineSlice::create("square02b_001.png");
    m_bg->setScale(0.5f);
    m_bg->setColor({0, 0, 0});
    m_bg->setOpacity(200);

    m_label = CCLabelBMFont::create("", "chatFont.fnt");

    addChild(m_bg);
    addChild(m_label);

    setScale(0.4f);
    m_bg->setOpacity(0);
    m_label->setOpacity(0);

    schedule(schedule_selector(TooltipNode::checkMouse));

    return true;
}
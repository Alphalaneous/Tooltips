#pragma once

#include <Geode/Geode.hpp>

using namespace geode::prelude;

namespace tooltips {

    class TooltipNode : public CCNodeRGBA {
    public:
        static TooltipNode* create();
        static TooltipNode* get();

        void addNode(CCNode* node);
        void removeNode(CCNode* node);

        void setLabel(ZStringView text);

        void show(float dt);
        void showTooltip(CCNode* node);

        void checkMouse(float dt);

    protected:
        bool init();

        std::set<CCNode*> m_activeNodes;
        CCNode* m_currentNode;

        CCLabelBMFont* m_label = nullptr;
        geode::NineSlice* m_bg = nullptr;
        float m_offset;
    };
}
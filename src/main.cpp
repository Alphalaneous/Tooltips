#include <Geode/Geode.hpp>
#include <Geode/ui/Button.hpp>
#include <alphalaneous.alphas_geode_utils/include/ObjectModify.hpp>
#include "TooltipNode.hpp"

using namespace geode::prelude;

class $baseModify(MyCCMenuItem, CCMenuItem) {
    void modify() {
        addOnEnterCallback([this] {
            tooltips::TooltipNode::get()->addNode(this);
        });
        addOnExitCallback([this] {
            tooltips::TooltipNode::get()->removeNode(this);
        });
    }
};

class $baseModify(MyButton, geode::Button) {
    void modify() {
        addOnEnterCallback([this] {
            tooltips::TooltipNode::get()->addNode(this);
        });
        addOnExitCallback([this] {
            tooltips::TooltipNode::get()->removeNode(this);
        });
    }
};

$on_mod(Loaded) {
    tooltips::TooltipNode::reset();
}

$on_game(TexturesLoaded) {
    queueInMainThread([] {
        tooltips::TooltipNode::reset();
    });
}
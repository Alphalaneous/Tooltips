#pragma once

#include <Geode/Geode.hpp>
#include <alphalaneous.alphas_geode_utils/include/Utils.hpp>

using namespace geode::prelude;

namespace tooltips::utils {

    inline Result<ZStringView> getReplacementForID(ZStringView ID) {
        static StringMap<std::string> replacements = {
            {"dankmeme.globed2/main-menu-button", "Globed"},
            {"cvolton.betterinfo/main-button", "BetterInfo"},
            {"techstudent10.gdguesser/start-btn", "GDGuesser"},
            {"robtop-logo-button", "RobTop's Website"}
        };

        auto iter = replacements.find(ID);
        if (iter != replacements.end()) {
            return geode::Ok(iter->second);
        }

        return geode::Err("Not found");
    }

    inline ZStringView getReplacementForWord(ZStringView word) {
        static StringMap<std::string> replacements = {
            {"id", "ID"},
            {"gdutils", "GDUtils"},
            {"gd", "GD"},
            {"youtube", "YouTube"},
            {"robtop", "RobTop"},
            {"hsv", "HSV"},
            {"ui", "UI"},
            {"tiktok", "TikTok"},
        };
        
        auto iter = replacements.find(word);
        if (iter != replacements.end()) {
            return iter->second;
        }

        return word;
    }

    inline void uppercaseFirstChar(std::string& str) {
        if (!str.empty()) {
            str[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(str[0])));
        }
    }

    inline std::string deduceExpectedName(CCNode* node) {
        auto replacement = getReplacementForID(node->getID());
        if (replacement) return replacement.unwrap();

        const auto& splitSlash = geode::utils::string::split(node->getID(), "/");
        if (splitSlash.empty()) return "";

        const auto& finalPart = splitSlash.back();
        auto splitDash = geode::utils::string::split(finalPart, "-");
		if (splitDash.size() == 1) {
        	splitDash = geode::utils::string::split(finalPart, "_");
		}

        std::string ret;
        for (auto& str : splitDash) {
            if (str == "button" || str == "btn" || str == "icon") continue;

            str = getReplacementForWord(str);

            uppercaseFirstChar(str);
            ret += str + " ";
        }
        if (!ret.empty()) ret.pop_back();
        return ret;
    }

    inline CCNode* getSceneChildContainingNode(CCNode* node) {
        if (!node) return nullptr;
        auto current = node;
        while (current && current->getParent() != CCScene::get()) {
            current = current->getParent();
        }
        return current;
    }

    inline bool isLastAlert(CCNode* node) {
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

    inline bool isHoverable(CCNode* node, CCPoint point) {
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

}